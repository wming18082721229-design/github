# Explainability 文献综述 · 组会报告

> 2026-07-09 · 覆盖 20 篇文献（Tier 1 精读 8 篇 / Tier 2 扫读 4 篇 / Tier 3 索引 8 篇）
> 项目锚点：**hybrid 检索（BM25 + SPLADE + Granite dense）× CorroborationReranker（反事实 NIAH）× 证据归因**

---

## 0. TL;DR（三句话版本）

1. **我们的定位站得住**：文献证明 citation 会被 post-rationalize（高达 57% 不忠实，#5），且检索质量是归因质量的主驱动（FEVER 上加 RAG 使 citation correctness 27%→77%，#7）——「把 faithfulness 防御做在检索/重排侧」这个空位目前没有人占。
2. **实现路径已抠清**：ALCE citation P/R 用 `google/t5_xxl_true_nli_mixture` 逐句判定（#4）；masking-and-recovery 用 LLM annotator 而非 NLI（GPT-4 α=0.65 vs NLI 0.29–0.34，#18）；calibrated parametric vote 照 ClashEval 的 percentile 校准配方（accuracy 0.615→0.754，#20）。
3. **三个必须提前准备的防御**：(a) rerank-only vs generation-only 消融（回应 Astute RAG）；(b) corroboration rerank vs 普通 cross-encoder rerank 的增益归因；(c) 我们自己的 answer extractor 也可能 post-rationalize，需要一个 Wallat 式注入实验自证。

---

## 1. 项目回顾（30 秒）

- **三条检索臂**：BM25（经典稀疏）+ SPLADE（学习型稀疏，词权重天生可解释）+ Granite dense（`granite-embedding-english-r2`），调优 convex 融合（α = dense 权重；RRF 已被淘汰）。
- **核心创新 CorroborationReranker**：对 top-20 候选，LLM 逐段抽取最短答案 + 一票 parametric answer；corroboration = 给出相同答案的其他来源数；`final = α·relevance + (1−α)·corroboration`。
- **评测**：自建 NIAH（needle + 真实语料挖掘的反事实 "Source-A" 干扰项；needle-found@k / MRR）+ SciFact/BEIR（nDCG/recall）+ NQ RAG（cover-EM/F1）+ 配对显著性检验。

---

## 2. 核心词汇表（来自三篇 survey，写作与答辩通用）

| 概念对 | 含义 | 出处 |
|---|---|---|
| **by-design（inherently interpretable）vs post-hoc** | 模型结构上透明（SPLADE 稀疏词权重）vs 对黑盒的事后解释（SAE 分解、occlusion、saliency） | #1 EXIR Survey |
| **fidelity vs plausibility** | 解释是否忠实于模型真实依据 vs 人类是否觉得合理——两者可以背离，评解释必须分开报 | #1 / #13 |
| **correctness vs faithfulness** | 引用「确实支持该句」（NLI 可判）vs 模型生成时「真的依赖了被引文档」（只能干预式测量）——正确的引用可以完全不忠实 | #5 |
| **in-generation（G-Cite）vs post-hoc（P-Cite）citation** | 解码时产生引用 vs 生成后补贴引用；G-Cite 精确但覆盖窄，P-Cite 覆盖广 | #2 / #7 |
| **post-rationalization** | 模型先用参数化记忆作答，再挑一篇字面吻合的文档贴引用——faithfulness 失效的典型机制 | #5 |
| **knowledge conflict（prior vs context）** | 模型内部先验与检索证据冲突时的仲裁问题；我们的 counterfactual Source-A 就是它的检索侧具象 | #19 / #20 |

---

## 3. Tier 1 精读 · 第一组：概念与立论（#5 / #19 / #20 / #7）

### #5 Correctness is not Faithfulness in RAG Attributions（Wallat et al., 2024）

- **能复用什么**：adversarial statement-injection 协议（把已生成答案的 statement 注入三类文档，看模型是否改引被污染文档）——正是我们 NIAH 反事实构造的 citation 侧镜像。四项 desiderata（Correctness / Appropriateness / Comprehensiveness / Faithfulness）是 related work 的干净概念框架。"faithful model should either cite itself or omit a citation" 可直接为我们的 parametric vote 背书。
- **精确协议**：Command-R+（104B, 4-bit）；NQ 1,444 题 + KILT Wikipedia；BM25 top-30 → ColBERT v2 → top-5；statement 级注入后字符串匹配判定，无 NLI、无人工。
- **我们哪里不同**：它证明生成端 citation 会 post-rationalize；我们把「parametric prior 与检索文档的一致性」变成重排阶段的**显式可数投票**，而非留给解码隐式决定。extraction prompt 的 "Using ONLY the passage" 约束是对 parametric leakage 的直接工程防御。
- **Reviewer 攻击点**：我们的 per-passage answer extraction 本身也是生成，同样可能 post-rationalize——**需要做一个 Wallat 式注入实验自证 extractor 不吃 token-match 诱饵**。且 parametric vote 与 corroboration vote 出自同一模型，"独立来源"叙事要收敛措辞。引用它时注意：单模型、单数据集、statement recover 率仅 63–70%，写成 "evidence of" 而非 "proof of"。

> 关键数字：注入 relevant-but-not-cited 文档被引率 **57%**（273/476）；random 文档也有 12%；cited-for-other-reason 55%。

### #19 Astute RAG（Wang et al., 2024）—— 我们最近的邻居，必须划清边界

- **能复用什么**：三个现成实验设计——① retrieval-precision 分桶性能曲线（我们可按「反事实占 top-k 比例」做同款）；② conflict 三分子集分析（both-correct / both-wrong / conflicting，正好评 "demote lone counterfactual" 能力）；③ RGB worst-case 全负例协议。立论引用：真实检索下 ~70% passage 不含真答案；conflict 案例中 internal/external 各对 47.4%/52.6% → **单靠多数票或单靠 parametric 都不够，必须 blend**。
- **精确协议**：自建 1,042 条 short-form QA（NQ/TriviaQA/BioASQ/PopQA）+ Google 真实检索 top-10；Claude 3.5 Sonnet / Gemini 1.5 Pro / Mistral ×2；string-match accuracy；默认 t=1、m̂=1。
- **我们哪里不同**：Astute 是**纯生成端**方案——conflict resolution 全部发生在一个 in-context consolidation prompt 里，信号隐式、不可分解；我们把同一直觉**下沉到 reranker**，corroboration 是显式算术，可离线 λ-sweep，改变的是 top-k 排序而非答案串——**两者正交可叠加**。它没有 reranking 概念，检索固定为 Google Search。
- **Reviewer 攻击点**：最锋利的一问——「生成端 consolidation 已能在 conflicting 子集 ~80% 选对，你 rerank 干预的增量何在？」**必须给 rerank-only vs generation-only vs both 消融**，并强调 needle 不进 top-k 时 consolidation 无从修复。第二问：frequency 投票会被重复出现的错误信息放大——NIAH 需加「多副本反事实」压力配置。

> 关键数字：Claude 上 Astute 61.7 vs 最强 baseline 58.8；worst-case 下 plain RAG 落后 No-RAG 50+ 点；conflict 率 19.2%。

### #20 ClashEval（Wu et al., 2024, NeurIPS D&B）—— 计划 (b) 的方法来源

- **能复用什么**：**Calibrated Token Probability Correction**——比较 r(q) 与 r(q|c) 的 mean token probability 的 **percentile**（原始值不可比：context-given 分布严重右偏），谁高用谁；可直接改造成 parametric vote 权重。Accuracy / Prior Bias / Context Bias 指标体系可映射为「prior 错时 needle 找回率」与「prior 对时 counterfactual 被顶上率」。分级扰动配方（数值 0.1–10×、年份 ±100、slight/significant/comical）可构造分级难度 counterfactual。
- **精确协议**：1,294 题 × 6 域，GPT-4o 生成并质检；6 个模型；主实验 k=1 单文档，附录 k=5；confidence = 无 context 回答的平均 token probability，10 bins。
- **我们哪里不同**：它是生成端**单文档二选一**仲裁；我们是检索端 **20 候选投票**，corroboration 计数天然稀释单个 counterfactual（它的 k=5 实验已暗示此效应，我们把它做成显式打分）。它的扰动是合成改写，我们的 Source-A 从真实语料挖掘——更 plausible，也更难防。
- **Reviewer 攻击点**：① 校准增益以 **prior bias 上升为代价**（0.021→0.085）——映射过来就是校准票会压制真 needle，必须报双向错误；② 「越 plausible 的扰动越难防」恰指向我们挖掘出的自然反事实，不能只在夸张干扰上报胜利；③ Granite 3B 的 logprob 校准性未知，**上手第一步是复现它的 Fig. 4 斜率图**。

> 关键数字：GPT-4o 在 prior 正确时仍采纳错误 context **60.8%**；校准修正 accuracy 0.615→**0.754**（context bias 0.304→0.107）；k=1→5 使误采 context 从 0.582→0.383。

### #7 Generation-Time vs. Post-hoc Citation（Saxena et al., 2025）

- **能复用什么**：立论引用 Finding 3——**"retrieval augmentation is the primary driver of citation accuracy"**（FEVER 上仅加 RAG 就使 G-Cite correctness 0.272→0.769）。五指标体系（Citation P / R / Correctness / Coverage / Latency）作为我们复现 ALCE 时的命名规范；human eval 设计（100 例/配置、2 annotator、κ=0.873）是小规模人评的规模论证模板。
- **精确协议**：LLaMA-3.1-8B-Instruct 统一四类方法（Zero-shot / Fine-tuned / RAG / Advanced）；ALCE、LongBench-Cite、REASONS、FEVER 四数据集做 cross-paradigm 适配；指标为与 ground-truth citation 匹配（非 NLI）。
- **我们哪里不同**：它评的是 attribution 附着环节（答案已定，问引用贴得对不对）；我们干预的是更上游的 candidate ranking——**它框架里两个 paradigm 都不覆盖的第三个位置（retrieval-time evidence selection）**，且无需 gold citation 标注即可跑在 NIAH 上。
- **Reviewer 攻击点**：「检索决定 attribution」是**必要非充分**——它测的是 correctness 匹配率，不是 Wallat 意义的 faithfulness，两个概念不能混用（正确写法：retrieval 提升 attribution 的 correctness 上限，faithfulness 需另测）。且 reviewer 会要求证明增益来自 corroboration 信号而非「rerank 本身」——**需要 vs cross-encoder rerank 的对照**。引用其数字要谨慎：workshop 短文无显著性检验，ALCE Zero-shot(P) precision=1.000 疑为协议 artifact。

> 关键数字：FEVER G-Cite correctness 0.272→0.769（+RAG）；human eval：P-Cite 答案正确率 78% vs G-Cite 69%。

---

## 4. Tier 1 精读 · 第二组：实现依据（#4 / #18 / #14 / #16）

### #4 ALCE（Gao et al., 2023, EMNLP）—— citation P/R 的照抄对象

- **能复用什么**：citation recall/precision 的原始定义与参考实现，逐字照抄：逐句切分 + NLI 判定 + 引用拼接规则 + irrelevant 双条件判定 + shortcut 哨兵（top-1 复读拿满 citation 分但 correctness 崩 → 三维度联合防作弊）。
- **精确协议（复现要点）**：
  - **NLI 判定器**：`google/t5_xxl_true_nli_mixture`（TRUE, T5-11B）。prompt 固定 `"premise: {P} hypothesis: {H}"`，输出 "1"=entail；passage 格式化 `"Title: {T}\n{text}"`，多篇 `\n` 拼接。
  - **粒度：逐句**。recall_i = 1 ⟺ 引用集非空 且 **所有被引 passage 拼接后**整体 entail 该句。
  - **precision**：引用 irrelevant ⟺ 单独不支持 **且** 去掉后其余仍支持；recall=0 时该句所有引用 precision 强制 0。已知弱点：**检测不了 partial support**（irrelevant 检测 precision 仅 66.1%）。
  - 语料切 100-word passages；每句最多 3 引用；ASQA 用 EM recall、QAMPARI 用 precision+recall-5、ELI5 用 LLM 生成 sub-claims + 同一 NLI。
- **我们哪里不同**：ALCE 判「该源是否支持此句」（NLI entail），我们判「该源给出的答案是否与他源一致」（字符串投票）——同在多源一致性上做文章，但作用位置（生成侧 vs 检索侧）和判定机制都不同。ALCE 无反事实设计。
- **Reviewer 攻击点**：NQ 短答通常单句，逐句协议退化为「整答案 vs 拼接引用」，粒度意义被质疑；TRUE 是 2022 年模型且不识别 partial support，拿它当金标准的可靠性会被问。

> 关键数字：ChatGPT ASQA citation recall/precision 73.6/72.5；自动分与人工 κ=0.698（recall）；shortcut 复读 citation 99.4 但 EM 仅 35.1。

**复现清单（NQ-RAG 上落地）**：① 加载 TRUE（fp16 约 45GB，HPC 预缓存；可先用 DeBERTa-v3-mnli 冒烟）；② RAG 输出带 `[k]` inline 引用；③ spaCy 切句 + 正则解析引用；④ 按上述 recall/precision 伪代码打分；⑤ correctness 直接用已有 cover-EM/F1；⑥ 加 top-1 复读哨兵基线。

### #18 masking-and-recovery（Xing, Baldwin, Lau, NAACL 2025）

- **能复用什么**：完整协议三步——**mask**（随机选一条证据，抹掉解释中所有引用它的 `[k]` marker）→ **recover**（annotator 拿证据 + masked 解释，找出所有本应引用它的句子）→ **set-level P/R/F1**。关键实证：**LLM annotator 显著碾压 NLI**（GPT-4 Krippendorff α=0.65 vs TRUE 0.34 / DeBERTa 0.29），因为 NLI 把句子抽离上下文。零 shot prompt 现成（附录 Table 13）。
- **精确协议**：PolitiHop 100 例；MTurk 68 人 × 每题 5 人 + 控制题质控；sample（每题 mask 1 条）与 full（逐条 mask，只有自动评测做得起）两种设置——**full 设置才能看出证据选择质量的影响**。
- **我们哪里不同**：它评多跳 fact-checking 的多句归因；我们要用它度量「corroboration 重排前后 → 下游归因质量」的因果链（对应它的 human-vs-machine evidence 对比）。annotator 换成 Granite。
- **Reviewer 攻击点（重要）**：**NQ 单句短答上协议退化**——X_cit 恒为单元素集，F1 非 0 即 1，无区分度；必须在多证据/多句 needle 场景使用。且 Granite 当 annotator 前需先对 GPT-4 或人工验证 α ≥ 0.5，否则自动分不可信。

> 关键数字：GPT-4 α=0.65（也只是 moderate）；即便人工挑证据，完全归因的解释也仅 **31%**；NLI annotator α 仅 0.29–0.34。

**复现清单**：① Granite 做 zero-shot recovery annotator（prompt 照 Table 13），先小规模对照 GPT-4 验证 α；② 每个检索 doc 当一条 evidence，full 设置逐条 mask；③ set-level P/R/F1 + Krippendorff α（`krippendorff` 包）；④ 跑「重排前 vs 后」两遍比较下游 attribution F1。

### #14 SciFact（Wadden et al., 2020, EMNLP）—— 闲置资产的精确定义

- **能复用什么**：gold rationale = **最小完备句集**（在 abstract 上下文中合起来足以让领域专家判定蕴含该 claim）；每 rationale ≤3 句（93% 是 1 句）、每 claim×abstract ≤3 组、组间互斥。abstract-level（correctly labeled / correctly rationalized）与 sentence-level F1 的定义可直接搬来评我们检索器的句级证据命中——**零新增标注**。
- **精确协议**：1,409 claims / 5,183 abstracts；三阶段 = TF-IDF top-3 → BERT 句选（sigmoid 阈值 0.5）→ BERT 三分类；句级评测要求「预测集包含某个完整 gold rationale」。
- **我们哪里不同**：我们目前只用第一阶段（abstract retrieval, nDCG/recall），第二、三阶段标注闲置；计划是拿 gold rationale 当句级金标准 zero-shot 评检索/重排（不训分类器）。它用 TF-IDF，正好被我们的三臂融合替换对比。
- **Reviewer 攻击点**：SciFact 每 claim 通常仅 1 个 evidence abstract、rationale 高度集中——**单源集中证据的数据集验证不了多源投票的价值**（所以 corroboration 的主战场只能是 NIAH，SciFact 负责句子级归因这条线）；且 rationale 只在 SUPPORTS/REFUTES 有，句级评测天然偏正例。

> 关键数字：VeriSci open 设置 sentence-level F1 仅 39.5；换 oracle rationale +20 点、再换 oracle abstract 又 +20 点——**检索是最大瓶颈**（对我们有利的立论）。

### #16 DeepSciVerify（Sadeghi et al., 2026）—— tie-breaker 推迟的证据

- **能复用什么**：cost-bounded escalation 完整先例：便宜信号先判，**仅当模型输出 NEI 才升级**到贵信号（无连续置信阈值，靠选保守模型实现校准）；两 LLM 按校准偏差分工（保守者做 trigger、平衡者做终判）。
- **精确协议**：SCitance 656 例（test 仅 91）；Phase 1 abstract 级 GPT-5.4 → NEI 才进 Phase 2 全文 RAG + GPT-4 终判；67% early-exit。
- **我们哪里不同**：它升级的是**证据粒度**（abstract→全文），我们设想的 tie-breaker 升级的是**判定信号**（答案投票→NLI entailment，仅对票数并列/分差小的候选）；我们单模型自托管，「校准互补」不成立，得用票数并列/分差阈值当 trigger。
- **Reviewer 攻击点 / 对我们的用法**：它的 escalation 净收益极薄——Phase 2 纠正 13、翻错 7，**净 +6/91 例**（+4.5 Micro-F1，显著性存疑）。**这正是我们推迟 tie-breaker 的最好引用**：escalation 收益有限，优先做投票主干。

> 关键数字：Micro-F1 86.7（+4.5）；67% early-exit；全文检索 p95 延迟 56.7s（长尾极重，量化了「只升级不确定案例」的动机）。

---

## 5. Tier 2 扫读（#9 / #10 / #13 / #8）

| # | 论文 | 定位 | 核心机制 | 对我们的直接用途 | 最硬的数字 |
|---|---|---|---|---|---|
| 9 | **Xetrieval** (2026) | post-hoc，SAE on embeddings | TopK-SAE（k=256）分解 dense embedding 为带自然语言描述的稀疏特征；reasoning internalizer（3 个 MLP 蒸馏 CoT embedding）可选；fidelity 用 **erase/keep-only 干预**验证 | 方向 2 模板：在 Granite dense 输出上训 SAE（FAISS cache 里的 embedding 可直接当训练数据）；erase/keep-only 协议是纯 embedding 算术，无需重训 | internalizer 使 e5-large NDCG@10 61.5→64.2；erase 解释特征降分最多、erase 非重叠特征反而升分 |
| 10 | **CERA** (2026) | inherently interpretable（训练注入） | KL(rationale‖CLS-attention) 辅助 loss + POS 加权 rationale mask（NOUN/VERB=1.0…DET=0.2）+ TextBlob subjectivity hard negatives | SciFact gold rationale 与其数据同构，评测协议（IOU-F1 / ERASER sufficiency）可复刻；subjectivity 排序思路可迁移到 counterfactual mining | Sufficiency 0.2073→0.0939（越低越好）而检索仅 ~0.01 波动；R@5 +0.13 |
| 13 | **Pandian** (2024, ECIR) | 评价协议（不提解释方法） | **MRC**：文档替换为 rationale 拼接后同模型重打分重排，与原排序算 Kendall's τ——**零标注的 fidelity 指标**；MER 需 sub-doc 标注 | 方向 2 评测骨架：同一协议比「SPLADE 内生词权重 vs dense occlusion 解释」的 MRC；其 chunk+max-pool 设定与我们 CONTRACT 3 同构，移植成本低 | 相关性最强 ≠ 最可解释：MonoElectra nDCG 最高但 MRC 输给 ColBERT；per-query MRC 与 nDCG 几乎不相关 |
| 8 | **FullCite** (2026) | 生成侧 attribution | 结构化 inline citation {doc_id, verbatim snippet}；三策略：prompt / FSA 约束解码 / **posthoc Jaccard-0.7 span 对齐** | 方向 3 的 span-gap 铁证 + posthoc 对齐是零训练组件，可直接进 RAGPipeline；其 primacy bias 发现与我们 needle-position 实验互证 | **BioASQ Doc-F1 58.08 vs Snippet-F1 6.18**（选对文档定位不到 span）；posthoc 对齐把 ASQA Snippet-F1 12.80→61.87 |

---

## 6. Tier 3 索引（8 篇，一行制）

| # | 论文 | 一句话价值 |
|---|---|---|
| 1 | EXIR Survey (2022) | `by-design vs post-hoc`、`fidelity vs plausibility` 词汇来源；pre-RAG |
| 2 | Attribution/Citation/Quotation Survey (2025) | 134 篇、300 指标/7 维的地图；「缺统一 benchmark」正是我们 NIAH+SciFact 组合的空档 |
| 3 | Text/IR XAI Survey (2022) | term-weighting 解释一节支撑「学习型稀疏 = 保留词项级可解释 + 语义匹配」 |
| 6 | MIRAGE (2024) | 模型内部信号（context-sensitivity + saliency）做归因——**需要白盒，我们自托管 Granite 是差异化条件**；备用验证手段 |
| 11 | Probing Ranking LLMs (2024) | 逐层探针看相关性形成；深挖层备选 |
| 12 | LLM Relevance MI (2025) | activation patching 三阶段通路（早层抽取→中层加工→晚层输出）；解释 listwise reranker 被骗时的方法论出处 |
| 15 | SciFact-Open (2022) | 语料扩到 500K 后 F1 掉 ≥15 点——与我们 `--max-docs` 规模扫描互证「失败边界随规模恶化」 |
| 17 | Step-by-step Med Verify (2025) | 迭代 QA 式可解释验证的代表；成本远高于我们 top_n+1 次调用，是可比优势 |

---

## 7. 综合：论文叙事链与差异定位

**立论三段式（每段有文献背书）**：

1. **问题**：citation 大量 post-rationalize（#5, 57%），模型过半会被错误 context 带偏（#20, 60.8%），即便人工挑证据、完全归因的解释也只有 31%（#18）→ 生成端归因不可信。
2. **杠杆**：检索是归因质量的主驱动（#7, +50pp）也是验证 pipeline 的最大瓶颈（#14, oracle 检索 +20 点）→ 干预应做在检索侧。
3. **空位**：冲突处理全在生成端（#19）、归因评测全在生成后（#4/#7/#18），**检索/重排阶段没有任何机制利用跨源一致性**→ CorroborationReranker 填这个位置，且收益能用纯检索指标（needle-found@k/MRR）干净归因，与生成器解耦。

**与最近邻的边界（一张表答辩用）**：

| | Astute RAG (#19) | ClashEval (#20) | 我们 |
|---|---|---|---|
| 作用阶段 | 生成端 consolidation | 生成端单文档仲裁 | **重排阶段投票** |
| 信号形式 | in-context prompt，隐式 | token prob 校准，二选一 | 显式计数 + convex blend，可 λ-sweep |
| 可测指标 | 只有答案 accuracy | accuracy / bias | **needle-found@k / MRR（检索指标）**+ 下游归因 |
| 反事实来源 | 真实检索噪声 | GPT-4o 合成扰动 | 真实语料挖掘（更 plausible） |

---

## 8. Reviewer 攻击点汇总与预案（按优先级）

| # | 攻击 | 来源 | 预案 |
|---|---|---|---|
| 1 | 「生成端 consolidation 已够好，rerank 增量何在？」 | #19 | **消融：rerank-only vs generation-only vs both**；强调 needle 不进 top-k 则生成端无从修复 |
| 2 | 「增益来自 corroboration 信号还是 rerank 本身？」 | #7 | **对照：vs Granite cross-encoder rerank**（TwoStageRetriever 已就位） |
| 3 | 「你的 extractor 也会 post-rationalize」 | #5 | 小规模 Wallat 式注入实验：往干扰段落塞 parametric 答案的 token-match，看 extractor 是否上当 |
| 4 | 「频次投票会放大重复的错误信息」 | #19 | NIAH 加**多副本反事实**压力配置 |
| 5 | 「校准 parametric 票压制真 needle（prior bias 上升）」 | #20 | 报双向错误（needle 找回率 + counterfactual 顶上率），不只报 needle-found@k |
| 6 | 「plausible 反事实最难防，别只在夸张干扰上报胜利」 | #20 | 用分级扰动配方构造难度梯度，报按难度分层的曲线 |
| 7 | 「masking 协议在单句短答上无区分度」 | #18 | 只在多证据/多句场景用；NQ 上主用 ALCE P/R |
| 8 | 「Granite 当 annotator/判定器可信吗？」 | #18 | 先对 GPT-4（或人工）验证 Krippendorff α ≥ 0.5 再采用 |
| 9 | 「SciFact 单源集中证据验证不了多源投票」 | #14 | 分工明确：corroboration 主战场 = NIAH；SciFact 只承担句子级归因线 |
| 10 | 「correctness ≠ faithfulness，不许混用」 | #5/#7 | 写作纪律：retrieval 提升的是 correctness 上限；faithfulness 用注入/干预实验另测 |

---

## 9. 下一步可执行清单（按依赖排序）

1. **[无依赖] 校准性冒烟测试**：复现 ClashEval Fig. 4——Granite 4.1-3b 无 context 答 NQ 子集，看 token prob 与正确率的斜率关系。校准性达标才做计划 (b)。
2. **[无依赖] 双向错误指标**：`eval/run_niah.py` 增加「prior 对时 counterfactual 顶上率」，与 needle-found@k 并报。
3. **[1 个 PR] 校准 parametric 票**：`corroboration_scores` 里把二值票换成 percentile-calibrated 权重（照 #20 配方）。
4. **[中等] ALCE citation P/R 落地**：TRUE 判定器（HPC 预缓存）+ RAG 输出带 `[k]` + 逐句打分 + 复读哨兵；先 DeBERTa 冒烟。
5. **[中等] 消融矩阵**：rerank-only / generation-only（Astute 式 prompt）/ both / cross-encoder 对照 —— 回应攻击 #1/#2。
6. **[低成本] SciFact 句子级评测**：gold rationale 当金标准，评三臂 + reranker 的句级证据命中（sentence-level F1 照 #14 定义）。
7. **[可选] fidelity 对比实验**（方向 2）：Pandian MRC 协议比 SPLADE 词权重 vs dense occlusion；Xetrieval erase/keep-only 干预做 Granite dense 的特征级验证。
8. **[推迟维持] entailment tie-breaker**：引 #16 的净 +6/91 作为推迟依据，写进 design doc 即可。

---

*详细四行制笔记（含全部关键数字与出处表号）见 `docs/reading_notes/`：`tier1_concepts_notes.md` / `tier1_impl_notes.md` / `tier2_scan_notes.md`；本报告为其组会浓缩版。*
