# Tier-1 精读笔记：归因/证据评测四篇

> 项目背景：IBM Granite NIAH 检索项目。三条检索臂（BM25 + SPLADE + Granite dense）convex 融合；核心创新 CorroborationReranker（LLM 从 top_n=20 候选各抽最短答案 + parametric 一票，corroboration = 相同答案的其他来源数，final = alpha·relevance + (1-alpha)·corroboration）。评测：自建 NIAH（needle + 反事实 Source-A 干扰项，needle-found@k / MRR）、SciFact/BEIR（nDCG/recall，目前只用 SciFact 第一阶段）、NQ RAG（cover-EM/F1）。计划：照 #4 复现 citation P/R、照 #18 复现 masking-and-recovery，作为「检索侧 corroboration 干预 → 生成侧归因质量提升」因果链的下游指标；SciFact gold rationale 用于句子级证据评测（零新增标注）；cost-bounded entailment tie-breaker 推迟中（#16 是先例）。

---

## #4 ALCE — Enabling LLMs to Generate Text with Citations (Gao et al., 2023, EMNLP)

- **能复用什么**：这是 citation recall / precision 的原始定义与参考实现，**必须精确照抄的一篇**。可直接复用：(1) 逐句切分 + NLI 判定的 citation recall/precision 算法；(2) 引用拼接 `concat(Ci)` 规则；(3) 「irrelevant citation」的双条件判定；(4) shortcut/cheating 鲁棒性检查（top-1 passage 复读会拿满分 citation 但 fluency/correctness 崩）。三维度评测（fluency=MAUVE、correctness、citation quality）的框架也可整体套到 NQ-RAG 上。判定器权重与 prompt 格式在 §C 给全。

- **精确实验协议**：
  - **Dataset**：ASQA（Wikipedia 21M，长问答/歧义 factoid，EM recall）、QAMPARI（Wikipedia，列表型 factoid，precision + recall-5）、ELI5（Sphere 899M CommonCrawl，why/how，claim recall）。各随机取 dev set 1000 例，无训练集（few-shot only）。语料切成 **100-word passages**（不是整页）。
  - **NLI 判定器（关键，逐字）**：`TRUE`，即 HuggingFace `google/t5_xxl_true_nli_mixture`，一个 **T5-11B** 模型，在 SNLI/MNLI/FEVER/SciTail/PAWS/VitaminC 上微调。Prompt 格式固定：`"premise: {PREMISE} hypothesis: {HYPOTHESIS}"`，输出 "1"=entail 否则 "0"。passage 作 premise 时格式化为 `"Title: {TITLE}\n{TEXT}"`，多 passage 用 `"\n"` 拼接。
  - **粒度**：**逐句（statement-level）**。输出按句子边界切成 statements s1..sn（QAMPARI 特例：list 里每个 entity 当一个 statement）。每 statement 最多允许 3 个引用（>3 无益）。
  - **Citation recall（每句 0/1，全句平均）**：句 si 的 recall=1 当且仅当 Ci≠∅ 且 `φ(concat(Ci), si)=1`，即**所有被引 passage 拼接后**整体 entail 该句。对齐 AIS 框架。
  - **Citation precision（每引用 0/1，全引用平均）**：先判某引用 ci,j 是否 "irrelevant"：irrelevant ⟺ (a) `φ(ci,j, si)=0`（单独不支持）AND (b) `φ(concat(Ci\{ci,j}), si)=1`（去掉它其余仍支持）。ci,j 得 precision=1 当且仅当 si 的 recall=1 且 ci,j 非 irrelevant。**recall=0 时该句所有引用 precision 强制=0**（前置条件）。注意：不检测 "partial support"，会误伤部分支持的引用（§E 明确承认此局限）。
  - **三数据集 correctness 指标差异（重点）**：ASQA=**EM recall**（dataset 提供的 short answers 是否为生成的精确子串，用 alias 规范化取最佳匹配）；QAMPARI=**precision + recall-5**（对 gold answer list 精确匹配，含 ≥5 正确即算 recall 100%）；ELI5=**claim recall**（用 `text-davinci-003` 从 gold answer 生成 3 条 sub-claims，再用同一 TRUE NLI 判定「模型输出是否 entail 每条 sub-claim」）。
  - **模型/prompt**：主实验 ChatGPT `gpt-3.5-turbo-0301`（4K 窗），另测 ChatGPT-16K、`gpt-4-0613`（8K）、LLaMA/Vicuna/LLaMA-2-Chat。VANILLA prompt = instruction + 2 in-context demos + top-k passages（每条 `Document [k](Title: ...) ...`）+ question。解码：ChatGPT/GPT-4 温度 0.5；开源模型 nucleus top_p=0.95。每配置 3 seeds 取平均。

- **我的做法哪里不同**：ALCE 是「生成侧」端到端 pipeline（retrieve→synthesize→cite），我的核心创新在**检索侧** CorroborationReranker（跨源答案投票重排）。ALCE 用 T5-11B TRUE NLI 判 entailment，我目前用 Granite LLM 抽最短答案 + 字符串投票（不做 NLI entailment）。ALCE 的 citation precision 靠 concat 去除法测「冗余引用」，我的 corroboration 是「多源同答案计票」——两者都在多源一致性上做文章，但 ALCE 判「该源是否支持此句」，我判「该源给出的答案是否与他源一致」。ALCE 无反事实干扰项设计，我 NIAH 显式注入 Source-A 反事实。

- **reviewer 会攻击哪一点**：(1) 逐句 + concat 的 recall 定义把「多句共享一个 needle」的情形算错——我 NQ-RAG 答案通常是**单句短答**，逐句协议在单句输出上退化为「整个答案 vs 拼接引用」，reviewer 会问逐句粒度是否还有意义；(2) TRUE T5-11B 是 2022 模型，precision 无法识别 partial support（ALCE 自己承认 false-positive 高，irrelevant 检测 precision 仅 66.1%），reviewer 会质疑用它当「归因质量金标准」是否可靠；(3) recall=0→precision=0 的强耦合会让 precision 数字在弱模型上被 recall 拖累，掩盖真实引用精度。

- **关键数字**：
  - ChatGPT VANILLA (5-psg) ASQA：citation recall **73.6** / precision **72.5**（Table 4）。ELI5 上约 **50%** 生成未被引用完全支持（citation recall ~51，Table 6）。
  - ALCE 自动分与人工分强相关：citation recall Cohen's κ=**0.698**、precision κ=**0.525**；以人工为金标准 citation recall 准确率 **85.1%**、precision **77.6%**（§6, §G.5）。
  - Shortcut 鲁棒性：Top-1 passage 复读 citation recall/precision **99.4/99.4** 但 EM 仅 35.1、MAUVE 20.8；First-2-sents citation **98.7/98.7** 但 correctness 18.9（Table 11）——证明三维度联合才防作弊。
  - RERANK（采样 4 条按 citation recall 选最佳）把 ASQA citation recall 73.6→**84.8**（Table 4）。
  - 检索是上限：GTR ASQA R@100=78.4，但模型 correctness 远低于此（Fig 4 / Table 12）——「即使答案在 context 里，LLM 也用不好」。

- **复现清单（在 NQ-RAG 输出上实现 ALCE citation P/R）**：
  1. **判定器**：加载 `google/t5_xxl_true_nli_mixture`（T5-11B，需 ~45GB fp16 或 8bit 量化；HPC 上 `MODEL_CACHE_DIR` 预缓存）。封装 `phi(premise, hypothesis) -> {0,1}`，prompt `"premise: {P} hypothesis: {H}"`，取 decode 首 token 是否 "1"。轻量替代：可先用 DeBERTa-v3-large-mnli 冒烟测试再上 T5-11B（见 #18 对比结论）。
  2. **输入格式**：RAG 生成需带 inline 引用 `[k]`（要么改 prompt 让 Granite 生成带引用文本，要么 post-hoc 用检索器给每句配引用）。passage 存为 `{doc_id, title, text}`；premise 拼接 `"Title: {title}\n{text}"` 用 `\n` 连接。
  3. **切句**：用 spaCy / nltk 把答案切成 statements；每句解析出 `Ci = [k...]`（正则 `\[(\d+)\]`），最多取 3。
  4. **打分伪代码**：
     ```
     for si in statements:
        Ci = parse_citations(si)
        recall_i = 1 if Ci and phi(concat([psg[k] for k in Ci]), text(si))==1 else 0
        for cij in Ci:
           if recall_i==0: prec_ij=0
           else:
              alone = phi(psg[cij], text(si))
              rest  = phi(concat(Ci\{cij}), text(si)) if len(Ci)>1 else 0
              irrelevant = (alone==0 and rest==1)
              prec_ij = 0 if irrelevant else 1
     citation_recall = mean(recall_i)          # 全句平均
     citation_precision = mean(prec_ij)         # 全引用平均
     ```
  5. **correctness（NQ 版）**：NQ 是短答 factoid，直接用 cover-EM/F1（已有）即可，无需 ALCE 的 sub-claim 生成；若要长答可仿 ELI5 用 LLM 生成 sub-claims + 同一 NLI 算 claim recall。
  6. **shortcut 哨兵**：加一个「top-1 passage 复读」基线跑同一套指标，确认你的三维度组合能把它的高 citation 分和低 correctness 分区分开。

---

## #18 Xing et al., 2024 — Evaluating Evidence Attribution in Generated Fact-Checking Explanations

- **能复用什么**：**citation masking-and-recovery 协议**的完整定义（mask/recover/打分三步）、set-level P/R/F1 公式、以及「LLM annotator > NLI annotator」的关键实证结论和现成 prompt（附录 Table 12/13/14）。对我们最有价值的是：它把归因评测从「NLI 逐句 entail」改成「给定一条证据、在整段解释里恢复出所有引用它的句子」的**多标签召回任务**，天然考虑了上下文（explicature），且证明 7B LLM 就能替代人工。sample vs full 两种评测设置也可借鉴。

- **精确实验协议**：
  - **Dataset**：PolitiHop（多跳政治 fact-checking），抽 100 例。每例 = claim + veracity label + 证据 passage 列表 E + human-selected evidence 子集。句子用 **spaCy v3.7.2** 切分。
  - **协议三步（逐字）**：
    1. **Citation Masking**：给定 claim c、veracity v、m 条证据 E={e1..em}、生成解释 X={x1..xn}（部分句含 inline 引用 [k]）。随机选一条证据 ek，把解释里所有引用它的 inline marker 抹掉（`you are wrong [6]` → `you are wrong`），得 masked 解释 X_mask；被抹的句集记 X_cit^mask（这是 ground truth，可能是**零/一/多句**）。
    2. **Citation Recovery（谁 recover）**：annotator（人工或 LLM）拿到 c、v、全证据集 E、被选证据 ek、masked 解释 X_mask，任务是**找出所有本应引用 ek 的句子** → 预测集 X_pred^mask。完美=两集相等。这是 multi-label 分类。
    3. **打分**：set-level Precision=|X_pred∩X_cit|/|X_pred|，Recall=|X_pred∩X_cit|/|X_cit|，F1=调和平均。每 claim 一组分，跨 claim 取均值。
  - **人工 annotator**：Amazon MTurk，68 人（final round）。每 claim 5 人标注。质控：pilot 每 HIT 含 3/6 控制题（2 正 1 负，负题正确答案是「无句可引用」），错任一即淘汰；final 用不重叠控制题，准确率 <70% 淘汰。付费 $2.70/HIT（~$15/hr）。多标签一致性用 **Krippendorff's alpha + Jaccard 距离**（kappa 不适用）。**每 claim 只 mask 一条证据**（sample 设置）。
  - **LLM annotator（谁来自动化 + 用什么 LLM）**：zero-shot prompt，输入 = 证据 ek + 全部解释句（编号），让 LLM 返回句号列表（prompt 见 Table 13：「Return the sentence number(s) separated by comma... Return -1 if none... Consider semantic citation relationships, not just keyword matching」）。测的 LLM：GPT-4 (`gpt-4-0613`)、GPT-3.5 (`gpt-3.5-turbo`)、LLaMA2 (`Llama-2-7b-chat`)，加小模型 LLaMA3.1-8B-Instruct、Mistral-7B-Instruct-v0.3、Gemma-1.1-7b-it（均 ≥8K 窗）。
  - **NLI annotator（对照）**：premise=证据 ek、hypothesis=解释单句，逐句配对，entailment 的句加入 X_pred。用 `deberta-v3-base` 和 `t5_xxl_true_nli_mixture`（同 ALCE 的 TRUE）。只取 entailment 类。
  - **两种评测设置**：**sample**（每 claim 只 mask 1 条证据，与人工对齐）、**full**（逐条 mask 每条证据，覆盖每个带引用句；每 claim 多组分取均值，人工做不到）。
  - **相关性数字（关键）**：LLM annotator 与人工的 Krippendorff alpha —— GPT-4 **0.65**（最高）、Mistral 0.59、Gemma 0.57、GPT-3.5 0.51、LLaMA2 0.52、LLaMA3.1 0.51；NLI 明显更低：TRUE **0.34**、DeBERTa **0.29**（Fig 4）。结论：**LLM annotator 显著优于 NLI**，因为 NLI 把句子抽离上下文，不可解释。
  - **另有维度**：Utility（解释多有用），用 Direct Assessment 0–100 滑条（非 Likert，因 Likert 均值有偏），可选 Bayesian 校准；Annotation Entropy 衡量标注不确定性。

- **我的做法哪里不同**：#18 评的是 fact-checking explanation 的**多句归因召回**（一条证据↔多句），我 NQ-RAG 通常单句短答，X_cit 基本是单元素集，masking-recovery 会退化。#18 用 PolitiHop 政治领域、人工/LLM annotator；我用自建 NIAH + NQ，判定器是 Granite。#18 的核心发现「human-selected evidence 产出更可归因的解释」正好对应我的假设「更好的检索（corroboration 重排）→ 更好的下游归因」——可直接借它当因果链的度量方法。

- **reviewer 会攻击哪一点**：(1) NQ 单句答案上 masking-recovery 退化为平凡任务（X_cit=单句），F1 恒 1 或 0，reviewer 会说这个协议对短答 RAG 没区分度——需论证在多证据/多句 needle 场景才有意义；(2) #18 用 GPT-4 当 annotator（alpha=0.65 也只是「moderate」），我若用 Granite 当 annotator，需先复现它的人工对照实验证明 Granite alpha 够高，否则「自动归因分」不可信；(3) #18 承认 PolitiHop 短句、可能与 LLM 参数知识冲突（LLM 忽略证据用内部知识），我 NIAH 反事实 Source-A 恰恰放大这个冲突，reviewer 会问「recover 出的句子是引用了证据还是复述了参数知识」。

- **关键数字**：
  - 人工评测 attribution F1：GPT-4 machine-evidence **0.74±0.31**（最佳）、human-evidence 0.63±0.29；GPT-3.5 ~0.52；LLaMA2 ~0.49（Table 3）。即便最好也仅 0.74，方差极大。
  - LLM-vs-human 一致性 Krippendorff alpha：GPT-4 **0.65** > 7B LLM(0.51–0.59) >> NLI(TRUE 0.34 / DeBERTa 0.29)（Fig 4）。
  - Fully-attributed proportion（全句 F1≥0.6 视为完全归因）：GPT-4 生成 + human evidence 仅 **31%**、machine evidence 仅 **9%**（GPT-4 annotator，Table 7）——「即便人工证据，也只 31% 解释全句归因准确」。
  - Machine-selected vs human-selected evidence 的检索质量（以人工证据为 gold）F1 仅 0.36–0.47（Table 4），但 sample 设置下生成归因质量差不多；**full 设置下 human evidence 明显更好**（Table 6/7）——说明必须逐句评（full）才看出证据选择的影响。

- **复现清单（在 NQ-RAG 输出上实现 masking-and-recovery）**：
  1. **判定器/annotator**：首选 **LLM annotator**（论文证明 >NLI）。用 Granite 4.1（本项目已有 LLMClient），zero-shot，prompt 照 Table 13：输入证据句 + 编号的解释句列表，输出逗号分隔句号 / -1。**先跑一个小规模人工对照（或用 GPT-4 当代理金标准）算 Granite 的 Krippendorff alpha**，达标（对齐 ≥0.5）才可信。
  2. **输入构造**：需要带 inline 引用 [k] 的多句解释（同 ALCE 步骤 2）。把每个检索到的 doc 当一条"evidence" ek。
  3. **masking**：正则找到引用 ek 的句子 → 记 X_cit；删掉这些句的 `[k]` marker 得 X_mask。full 设置：对每条 ek 各做一次。
  4. **recovery + 打分伪代码**：
     ```
     for ek in evidence_set:                     # full 设置
        X_cit = {i for i,sent in enumerate(sents) if k in citations(sent)}
        X_masked = strip_marker(sents, k)
        X_pred = llm_recover(ek, X_masked)        # Granite 返回句号集
        P = |X_pred & X_cit| / |X_pred|  (0 if empty)
        R = |X_pred & X_cit| / |X_cit|   (skip if X_cit empty)
        F1 = harmonic(P, R)
     attribution_F1 = mean over ek over claims
     ```
  5. **一致性度量**：用 Krippendorff alpha + Jaccard 距离（`simpledorff` 或 `krippendorff` 包）对比 Granite-annotator 与人工/GPT-4。
  6. **因果链接线**：跑两遍——corroboration 重排前 vs 后的检索结果各喂生成器，比较下游 attribution F1（+fully-attributed proportion），验证「检索干预→归因提升」。这正是 #18 的 human-vs-machine evidence 对比的迁移用法。

---

## #14 SciFact — Fact or Fiction: Verifying Scientific Claims (Wadden et al., 2020, EMNLP)

- **能复用什么**：**gold rationale 句级标注**（每 claim×abstract 的最小完备证据句集）——正是我「零新增标注做句子级证据评测」的数据来源。三阶段任务定义（abstract retrieval → rationale selection → stance/label）与 abstract-level / sentence-level F1 的精确定义可直接搬来评 SPLADE/Granite 检索器在证据句粒度上的表现。VeriSci 三段式 pipeline 是我检索器接 rationale selection 的现成范式。

- **精确实验协议**：
  - **Dataset**：1409 claims / 5183 abstracts。label 三分类 SUPPORTS/REFUTES/NOINFO（分布 556/337/516）。Train 809 / Dev 300 / Test 300。claim 来自 citances（引用句改写的 atomic claim），REFUTES 由 NLP 专家写否定（避免 "not" 类关键词偏差）。
  - **gold rationale 标注规则（重点）**：rationale = 「一组句子，作为 abstract 上下文中的前提合起来能被领域专家合理判定为蕴含该 claim 的**最小句集**」。每个 SUPPORTS/REFUTES 的 claim×abstract 对须标出**所有** rationale。统计（Table 2b）：**每 rationale ≤3 句**（1542 个 1 句 / 92 个 2 句 / 11 个 3 句）；**每 claim×abstract ≤3 个 rationale**（552 个 1 rationale / 290 个 2 / 153 个 3）；**每 claim 的 evidence abstract 数**多为 1（830 个 1 / 37 个 2 / 26 个 3+，516 个 0=NOINFO）。**多个 rationale set 互斥（mutually exclusive）**，28 个含非连续句。质控：label 一致性 Cohen's κ=0.75，句级 rationale κ=0.71。
  - **三阶段形式化**：输入 claim c + corpus A。系统输出预测证据 abstract 集 Ê(c)；对每个 a∈Ê(c) 预测 label ŷ(c,a) 和一组 rationale 句 Ŝ(c,a)。注意**预测只需一组句子**（可跨多个 gold rationale），但 gold 可有多组。
  - **Abstract-level 评测（仿 FEVER score）**：预测 abstract a **correctly labeled** ⟺ (1) a 是 gold evidence abstract 且 (2) ŷ=y。**correctly rationalized** ⟺ 再加：预测句集**包含某个完整 gold rationale**（∃ gold Ri ⊆ Ŝ）。限预测 ≤3 rationale 句。指标 = correctly-labeled / correctly-rationalized abstract 的 micro-F1，记 `Label-Only` 和 `Label+Rationale`。
  - **Sentence-level 评测**：预测句 ŝ **correctly selected** ⟺ (1) ŝ∈某 gold rationale Ri 且 (2) **同一 Ri 的其余句也全在 Ŝ 里** 且 (3) ŷ≠NOINFO。**correctly labeled** ⟺ 再加 ŷ=y。指标 = micro-F1，记 `SentenceSelection-Only` / `SentenceSelection+Label`。句级**不限**预测句数（本身惩罚 over-predict）。
  - **两种运行设置**：Open（FEVER 式，须自己检索 abstract）、Oracle-abstract（ERASER 式，给 gold evidence abstract）。
  - **VeriSci 基线结构（BERT-to-BERT 三段 pipeline）**：
    1. **ABSTRACT RETRIEVAL**：TF-IDF（unigram+bigram）取 top **k=3** abstract。
    2. **RATIONALE SELECTION**：BERT 编码 `[sentence, SEP, claim]`，sigmoid 打分 z̃i，阈值 t（SciFact 训练用 0.5；FEVER 迁移用 t=0.025，UKP Snopes 用 0.75）选句。训练用 SciFact-only 最佳。
    3. **LABEL PREDICTION**：BERT 编码 `[ŝ1..ŝℓ, SEP, claim]` 三分类 softmax。训练用 FEVER→SciFact 微调最佳。无 rationale 句则预测 NOINFO。
    - 编码器 RoBERTa-large（两段都用它整体最佳），SciBERT 在 rationale selection 略优。
  - **模型/训练**：HuggingFace Transformers，P100 单卡。rationale selection RoBERTa-large lr 1e-5(base)/1e-3(linear)，batch 256（梯度累积），cosine decay 20 epoch。

- **我的做法哪里不同**：SciFact 三阶段做的是「检索+句选+立场分类」全 pipeline，我目前**只用 SciFact 第一阶段（abstract retrieval，nDCG/recall）**跑我的三臂融合检索器。SciFact rationale selection 是监督 BERT 分类器，我若用它是想拿 gold rationale 当**句级证据金标准**评检索器/reranker 能否把 rationale 句排上去（无监督/zero-shot），不训练分类器。SciFact 用 TF-IDF 检索，我用 BM25+SPLADE+Granite convex 融合——可直接对比替换 abstract retrieval 那一段。

- **reviewer 会攻击哪一点**：(1) SciFact 每 claim 通常只有 1 个 evidence abstract、rationale ≤3 句、多为 1 句，**证据高度集中**，与我 NIAH「多源 corroboration」场景相反——reviewer 会问「单源集中证据的数据集能否验证多源投票的价值」；(2) 我只用第一阶段 retrieval，抛掉了 label/rationale 阶段，reviewer 会质疑「检索 recall 高 ≠ 下游证据/立场对」（SciFact 自己 Fig：oracle 检索后仍差 20+ 点，见下）；(3) rationale 是 SUPPORTS/REFUTES 才有，NOINFO 无 rationale，句级评测天然偏向有证据的正例。

- **关键数字**：
  - VeriSci Open 设置 test：SentenceSelection+Label F1 **39.5**、AbstractLabel+Rationale F1 **46.5**（Table 4 Row 6）。相比 FEVER-only zero-shot（Row 5：26.9 / 36.4）分别相对提升 **47%** 和 **28%**（in-domain 数据的价值）。
  - 三阶段误差均摊：换 oracle rationale 句级 +~20 点，再换 oracle abstract 又 +~20 点（Row 6→4→1）——**检索是最大瓶颈之一**。
  - Oracle-abstract 设置 VeriSci：AbstractLabel+Rationale F1 **72.7**（Row 3），端到端等效分类准确率 ~70%。
  - Rationale selection 训练数据消融（dev）：SciFact-only F1 **72.1** > FEVER+SciFact 69.7 > UKP Snopes 50.5 > FEVER 48.4（Table 3）。Label prediction 则 FEVER+SciFact 准确率 **81.9%** 最佳。
  - Claim-only 基线准确率仅 44.5%（Table 3）→ 否定过程未引入明显 artifact。
  - COVID-19 案例：36 claim 中 23 个 VeriSci 输出被专家判为 plausible。

---

## #16 DeepSciVerify — Claim–Citation Alignment via LLM-Driven Evidence Escalation (Sadeghi et al., 2026)

- **能复用什么**：**cost-bounded 两阶段 escalation** 的完整先例（正是我推迟中的 entailment tie-breaker 的参照）——「先便宜信号判定，仅对不确定案例升级到贵信号」的设计模式、触发条件、双 LLM 角色分工、成本/latency 量化都可直接借鉴。它的「用不同 LLM 的互补校准偏差分配角色」思路也可迁移到我的 reranker（保守模型做初筛、平衡模型做终判）。

- **精确实验协议**：
  - **Dataset**：SciTance（Alvarez 2024，源自 SciFact，用 citance 当 claim；CONTRADICTS 由 GPT-3.5 否定 citance 生成）。656 例（251 SUP / 225 CON / 180 NEI），split 467 train / 98 dev / 91 test。三分类 SUPPORTS/CONTRADICTS/NEI。
  - **两阶段 + 触发条件（重点）**：
    - Phase 1（abstract-level）：解析 citation→多源检索 abstract→LLM verifier fa 判 {SUP,CON,NEI}。**触发升级的条件极简单：ŷa == NEI 才升级**；ŷa∈{SUP,CON} 直接 early-exit 返回。**没有连续置信度阈值**——用「模型输出 NEI」本身当不确定信号（靠选一个「保守/爱输出 NEI」的模型来实现校准）。
    - Phase 2（passage-level）：仅对 NEI 案例，检索全文→RAG 抽 passage（chunk→embed→cosine top-k）→LLM verifier fp 终判 {SUP,CON,NEI}。
  - **两个 LLM 角色分工（关键设计依据）**：靠 calibration analysis 选角色。Phase 1 verifier = **GPT-5.4**（保守、高 NEI recall 81%、爱 defer，适合决定「是否升级」）；Phase 2 verifier = **GPT-4**（三类最平衡，SUP/CON/NEI recall 81/86/74，适合终判）。retrieval fallback 用 gpt-5.2，RAG embedder 用 `text-embedding-3-small`。全部 temperature 0，abstract verifier reasoning effort=low。
  - **retrieval 级联**：abstract=ID→DOI→title→fallback（arXiv/S2/CrossRef/OpenAlex/PubMed/web），title 相似度门槛 τ=0.30；full-text=URL→arXiv→OA→web-search fallback，须 ≥1500 字符且过滤 erratum。passage：chunk 3000 字符 overlap 200，cosine 阈 0.50，top-k=2。
  - **prompt 结构**：JSON 输出 `{"verdict": "SUPPORTS|CONTRADICTS|NOT_ENOUGH_INFO", "reasoning": "..."}`；Phase 2 明确「PASSAGE 是主证据，ABSTRACT 只作补充上下文，不得覆盖 passage」。

- **我的做法哪里不同**：DeepSciVerify escalation 是**证据粒度升级**（abstract→full-text passage），我的 cost-bounded tie-breaker 设想是**判定信号升级**（便宜的答案投票 corroboration → 贵的 NLI entailment 仅对并列/低置信候选）。它用「模型吐 NEI」当触发，我需自定义置信度（如 corroboration 票数并列、top-1/top-2 分差 < 阈值）当触发。它用两个不同商用 LLM 分角色，我是单一 Granite 自托管——校准互补性不成立，得靠温度/prompt 或阈值实现「保守初筛」。

- **reviewer 会攻击哪一点**：(1) 触发条件 = 「模型输出 NEI」把校准责任完全推给单个模型的偏差，reviewer 会问「这不是原则性的置信度，换个模型就失效」（论文自己承认 GPT-5.4 单阶段用会掉分，只是碰巧适合当 trigger）；(2) test 仅 **91 例**，+4.5 Micro-F1 的显著性存疑（论文自己提醒 caution）；(3) escalation 净收益薄（Phase 2 纠正 13、新引入 7 错，净 +6），reviewer 会问成本换来的收益是否值得——这正是我推迟 tie-breaker 的理由，可引用它当「escalation 收益有限」的证据。

- **关键数字**：
  - DeepSciVerify 三分类 test Micro-F1 **86.7** / Macro-F1 81.5，比最强 abstract-only 基线 (GPT-4 82.2) **+4.5**，比 Alvarez 2024 最佳 **+6.6**（Table 3）。
  - **67.0%** 实例在 Phase 1 early-exit 解决（61/91，其中 57 正确），仅 **33.0%** 升级（Table 6 / §D）。
  - Escalation 净效应：Phase 2 纠正 13、正确保留 NEI 9、翻错 7 → **净 +6 正确**（对应 +4.5 Micro-F1）。
  - Extractor 消融：RAG extractor +4.5 Micro-F1（全指标齐升）；LLM extractor 仅 +1.1 且 Sup./Not-Sup. 从 86.7 **降到 83.3**（过度预测 SUPPORTS）（Table 4）——RAG 比 LLM 直抽更可靠。
  - Latency：abstract retrieval 中位 4.10s（p95 6.89s）；full-text 中位 1.96s 但 **p95 56.70s、max 97.76s**（长尾极重）——量化了「只升级不确定案例」省成本的动机（Table 7）。
  - 校准偏差（Fig 3，train+dev n=565）：GPT-5.4 把 33% 真 SUPPORTS 判成 NEI；Claude Sonnet 4.6 把 34% 真 NEI 判成 SUPPORTS（过度自信）。

---

## 判定器选型速查（跨篇汇总）
| 用途 | 论文 | 判定器 | 备注 |
|---|---|---|---|
| citation entail (P/R) | #4 ALCE | `google/t5_xxl_true_nli_mixture` (TRUE, T5-11B) | prompt `premise:{}hypothesis:{}`→"1"；无 partial support |
| citation recovery annotator | #18 Xing | **LLM (GPT-4 最佳, alpha 0.65)** >> NLI (TRUE 0.34 / DeBERTa 0.29) | LLM 因带上下文胜出；Granite 需先验证 alpha |
| rationale selection / stance | #14 SciFact | RoBERTa-large / SciBERT (监督) | gold rationale ≤3 句、≤3 组、互斥 |
| claim-citation verify | #16 DSV | GPT-5.4(保守初筛)+GPT-4(平衡终判) | 触发=输出 NEI；RAG>LLM 抽取 |
