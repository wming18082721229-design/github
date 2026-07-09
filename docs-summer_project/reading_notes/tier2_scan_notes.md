# Tier-2 扫读笔记（abstract + method）

对应候选方向 2（hybrid 两臂对称可解释性 + fidelity 验证）与方向 3（SciFact gold rationale 句子级归因、span-gap 量化）。

---

## #9 Cai et al. 2026 — Xetrieval: Mechanistically Explaining Dense Retrieval
arXiv:2605.29507 | Beihang + BIGAI

### 一句话定位
**post-hoc、embedding-level 机制解释**：不改动 off-the-shelf dense retriever，在其输出 embedding 上训练 SAE（sparse autoencoder）做事后分解——但通过一个额外训练的 "reasoning internalizer" 预处理 embedding，使其更易被稀疏分解，处于 post-hoc 与"重塑表示空间"之间的中间地带。

### 核心机制
1. **Reasoning internalizer**：是 3 个 aspect-specific 的单隐层 MLP（tanh 激活），aspects = {SUMMARY, PURPOSE, QA}。训练方式：从 StackExchange 取文档，用 LLM（DeepSeek-V3 等）为每篇文档生成 3 种 aspect 的 CoT reasoning 文本，用**同一个 dense encoder** 编码得到 reasoning target embedding z^(t)；MLP 以 MSE loss 学习从 raw embedding z 直接映射到 z^(t)。推理时一次 forward pass 即可近似 CoT-enhanced embedding，绕开自回归生成（成本随文档数线性增长的 CoT reasoner 被摊平为常数级）。
2. **Mechanistic explainer = SAE**：对 reasoning-enhanced embedding 做字典学习分解 c=g(x), x̃=Wc+b，loss = 重构 MSE + λ·稀疏惩罚。系统比较了 7 种 SAE 变体（ReLU/TopK/BatchTopK/Gated/JumpReLU/P-Anneal/GatedAnneal），最终选 **TopK-SAE, k=256**（在 reconstruction error / mono-semanticity / retrieval retention 三轴权衡最优）。
3. **特征的自然语言描述**：自动化 pipeline（Paulo et al. 2024 / Park et al. 2025 风格）——对每个 active feature 取 top-activating 样本，LLM 总结成一句 semantic hypothesis h_j。
4. **单次检索决策的解释**：query 侧算 sparse code 并二值化 a_q,j = I[c_q,j > τ]；document 侧构造多视图 V(d) = {raw embedding} ∪ {3 个 internalized reasoning views}，对每个视图算激活，按 max 聚合；解释 E(q,d) = query 与任一 doc 视图**共同激活**的特征集合 O(q,d) 及其自然语言描述。多视图聚合能暴露 raw embedding 中纠缠/微弱的相关性特征。
5. **Fidelity 验证（三层）**：
   - **Detection Score**（描述质量）：给 LLM 一组 activating + non-activating 句子和 feature hypothesis，让其判断每句是否符合假设，分类准确率即分数；对比 Random SAE / Raw SAE，reasoned-embedding 版显著更高。
   - **Pair-level intervention（局部归因）**：在 doc embedding 中 erase / keep-only 解释特征张成的分量，看 cosine similarity 变化——erase Xetrieval 特征降分最多；erase non-overlap active 特征反而常**升分**（说明那些是干扰特征）。
   - **Task-level steering（全局效用）**：用 RUS(f_j) = Σ_pos I_j − Σ_neg I_j（正负 pair 上的 contrastive co-activation 频次）选 key features，解码前对其激活乘缩放因子 α，α>1 放大 → NDCG@10 提升，α<1 抑制 → 明显退化；non-key features 影响小。

### 对我们的直接用途
- **方向 2 的核心模板**：可在 Granite dense arm（granite-embedding-english-r2 的 L2-normalized 输出）上训练 TopK-SAE，把 dense 分数分解为共同激活稀疏特征——与 SPLADE 的天然词权重解释形成"事后 vs 内生"对称对比；`src/retrieval/embedder.py` 输出可直接作为 SAE 训练数据（SciFact/NQ corpus embedding 已有 FAISS cache）。
- 其 **erase/keep-only intervention 协议**可直接搬来做我们的 fidelity 验证实验（干预后重算 FaissIndex 分数即可，纯 embedding 算术，不需重训模型）。
- Reasoning internalizer 部分可选做（成本高，需 LLM 生成监督数据）；仅 SAE + intervention 已构成完整可发表实验。
- 注意其 Limitations 自述：只在 sentence-embedding 输出层，未探测模型内部 circuits；SAE fidelity 有限（不如 Transcoder）。

### 关键数字
- TopK-SAE k=256（L0=256）：保持强 mono-semanticity 同时 retrieval retention 接近 baseline（NDCG@10 baseline 11.0，见 Fig.3）。
- Reasoning internalizer 提升检索（NDCG@10 avg）：e5-large 61.5→64.2（CoT reasoner 66.5，即 internalizer 恢复约一半增益）；snowflake 38.3→51.2（CoT 54.7）。
- 效率：CoT reasoner 解释延迟随文档数近似线性增长（至 ~10^3-10^4 秒 @60k docs），Xetrieval 单次 feed-forward 几乎恒定（<10^0 秒量级）。

---

## #10 Vargas et al. 2026 — CERA: Contrastive Evidence Retrieval with Interpretable Attention Alignment
arXiv:2606.01482 | GitHub: franciellevargas/CERA

### 一句话定位
**inherently interpretable（训练时注入）**：fine-tune 阶段用人工标注 rationale 直接监督 CLS-to-token attention，把"解释"做进模型内部，而非事后归因——自称首个结合 subjectivity-based hard negative + attention alignment 的检索框架。

### 核心机制
1. **数据**：Evidence Inference 2.0（3,393 篇临床试验报告，1,916 个 ICO 查询），每条有专家标注的 gold evidence span [s,e]（字符级连续区间）。文档切成定长 chunk（chunk_size ≈ median(标注长度) + σ），与 gold span 有 overlap 的 chunk 为 positive。
2. **Hard negative 选择（subjectivity-based）**：不重叠 gold span 的 chunk 按 **TextBlob subjectivity score** 排序，取 top-K=5 最"主观"的 chunk 作 hard negatives——直觉是主观性内容与 evidential 内容词汇分布不同，比传统按 relevance 分挑负例更能教模型区分"话题相关但非证据"。
3. **Rationale mask 构造（POS 加权）**：positive chunk 内 token 级指示 r_j = I[token 在 gold span 内]，再乘 POS-dependent 权重 w(t_j)（spaCy 标注）：NOUN/PROPN/VERB=1.0, ADJ=0.9, ADV=0.8, NUM=0.7, PRON/AUX=0.4, ADP=0.3, DET/CCONJ/SCONJ=0.2, PUNCT=0.0。得 r̂_j = r_j·w(t_j)，归一化为分布 r̃。
4. **Attention 抽取与对齐 loss**：取最后一层 [CLS]→token attention，跨 H 个 head 平均后 softmax 得 ã；对齐目标 L_alignment = KL(r̃ ‖ ã)。
5. **双目标 loss**：L = L_triplet + λ·L_alignment。L_triplet = max(0, sim(q,C−) − sim(q,C+) + m)，margin m=0.2，cosine over L2-normalized [CLS] embedding（backbone: facebook/contriever, lr=1e-6, bs=8, 10 epochs）。λ∈{0.01, 0.05}；λ 越大 faithfulness 越好但 early-rank 检索指标略降。
6. **解释评价**：plausibility 用 IOU-F1 / Token-F1（对 gold rationale），faithfulness 用 ERASER 的 comprehensiveness / sufficiency。

### 对我们的直接用途
- **方向 3 直接对标**：SciFact 自带句子级 gold rationale（evidence sentences），结构与 Evidence Inference 完全同构——可复刻其协议：把我们各臂（BM25 词权重 / SPLADE token 权重 / dense 事后归因）的 token/句子归因对 SciFact rationale 算 IOU-F1 / Token-F1（plausibility）+ comprehensiveness/sufficiency（faithfulness）。
- POS 加权 rationale mask 的权重表可直接借用；TextBlob subjectivity hard-negative 思路对我们的 NIAH counterfactual distractor 挖掘（`src/niah/counterfactual.py`）有参考价值——"主观性"可换成"反事实性"作为 hard negative 排序信号。
- 若做 fine-tune 版实验，其 KL(rationale‖attention) 辅助 loss 是最小改动方案（只加一项 loss，检索性能仅 ~0.01 波动）。
- 局限：单一临床数据集、局部（同文档内 chunk 池）评测，泛化性未证。

### 关键数字
- 检索（vs HardNeg baseline）：Recall@1 +0.0768（0.1248→0.2016），Recall@5 +0.1307（0.4301→0.5608），MRR +0.1009（0.3288→0.4296）；vs 原始 Contriever 更悬殊（R@1 0.0214→0.2016）。
- Faithfulness：Sufficiency 0.2073（CERA 无对齐）→ 0.0939（λ=0.05，越低越好）；Contriever base 为 0.4273。IOU-F1 0.1393→0.1701。
- 对齐的代价：λ 0.01→0.05 时 Recall@1 0.1898→0.1747（early-rank 指标下降，高 cutoff 稳定）。

---

## #13 Pandian, Ganguly, MacAvaney 2024 — Evaluating the Explainability of Neural Rankers
arXiv:2403.01981 (ECIR) | GitHub: saranpandian/XAIR-evaluation-metric

### 一句话定位
**评价协议论文（不提出解释方法）**：固定同一个 post-hoc occlusion 解释算法，横向量化"每个 ranker 有多可解释"，给出 intrinsic（无需人工标注）+ extrinsic（需 sub-document relevance）两个指标——正是我们要的 fidelity/consistency 操作化定义。

### 核心机制
1. **协议**：每个 IR 系统除返回 top-k 文档外，还须为每篇文档返回 m 条 rationale（任意文本片段）。解释算法统一用 **occlusion**：从文档采样 n 个 segment（句子或长度 w 的词窗），mask 后算相对分数变化 φ(d_i) = |θ(Q,D) − θ(Q,D∖segments)| / θ(Q,D)，多轮采样累积权重，取 top-m 为 rationales。文档级检索用贪心逐句 occlusion（选 φ 最大句、移除、重复 m 次）。
2. **Intrinsic — MRC（Mean Rank Correlation，= consistency/fidelity）**：把每篇文档替换为其 m 条 rationale 的拼接 D^(i)，用**同一个黑盒模型**重新打分并重排 top-k，与原始排序算 Kendall's τ，对 query 平均。MRC 高 = 模型只看 rationale 也给出相似排序 = 解释忠实。**不需要任何人工标注**。
3. **Extrinsic — MER（Mean Explanation Relevance，= plausibility/correctness）**：需要 sub-document 级相关性标注（他们用 MS-MARCO QnA 的 URL/文本匹配把 passage 判定映射进 document corpus）。MER = 每条 rationale 与该文档所有 relevant passage 的 max cosine 相似度，对 (query, doc, rationale) 平均。
4. **实验设定**：TREC DL'20（54 queries），BM25 一阶段 top-1000 + NRM 重排（ColBERT / TCT-ColBERT / MonoT5 / MonoElectra），文档切 3 句 chunk、max-pool 聚合（与我们 build_run 的 CONTRACT 3 完全一致）。
5. **主要发现**：相关性最好的模型 ≠ 最可解释（MonoElectra nDCG 最高但 MRC@10 不敌 ColBERT）；句子级 rationale 比词窗更 consistent；passage 任务上 BM25 的词窗解释比所有 NRM 都 consistent；per-query 上 MRC 与 nDCG@10 几乎不相关（MonoT5 甚至负相关）→ 可解释性是与相关性正交的评价维度。

### 对我们的直接用途
- **方向 2 的评测骨架**：MRC 可直接实现于我们的框架——对 SPLADE：rationale = top 权重 term，重打分 = SparseIndex 上只保留这些 term；对 Granite dense：rationale = occlusion 选出的句子，重打分 = 重新 embed 拼接文本过 FaissIndex。同一协议下比较「SPLADE 内生词权重解释 vs dense 事后 occlusion 解释」的 MRC，即回答"哪臂更可解释"。
- MER 在 SciFact 上比他们更干净：SciFact 自带句子级 gold rationale，无需近似映射，ω 可以用精确句子匹配替代 cosine。
- 其 chunk(3句)+max-pool 设定与 `eval/run_benchmark.py` 的 build_run 完全同构，移植成本低；occlusion 只需调用现有 retriever 的打分接口，无需训练。
- 参数建议：m（rationale 数）增大会机械抬高 MRC（趋近全文）但降低 MER，取 m=1~3、句子粒度较稳。

### 关键数字
- Passage 任务（sentence rationale, m=1）：MRC@10 — ColBERT 0.4502 > MonoElectra 0.4165 > BM25 0.4000 > MonoT5 0.3481 > TCT-ColBERT 0.2938；而 nDCG@10 排序是 MonoElectra 0.7460 最高 → 二者解耦。
- 词窗 rationale 时 BM25 MRC@10=0.3029 反超最好 NRM（ColBERT 0.2790）。
- Document 任务 extrinsic：MER@10 最高是 TCT-ColBERT 0.2069（intrinsic 最高却是 MonoElectra 0.2493）→ intrinsic 与 extrinsic 也互不相关。

---

## #8 Yeginbergen et al. 2026 — FullCite: Explicit Evidence Grounding via Structured Inline Citation Generation
arXiv:2606.07130 | UPV/EHU + ITU Copenhagen

### 一句话定位
**生成侧的 attribution（非检索侧解释）**：让 LLM 在每条 claim 后生成结构化 inline citation {doc_id, verbatim snippet}，同时做 document 级和 evidence-span 级归因；系统量化了「选对文档 ≫ 定位对 span」这一 gap。

### 核心机制
1. **输出格式**：每条 claim 后跟 `{doc_id: <id>, snippet: <verbatim text>}`，snippet 限 20–512 字符。三种策略：
   - **Prompt-based**：纯指令要求 verbatim 引用，无任何解码干预——格式与 verbatim 性全靠模型 instruction-following。
   - **Constrained decoding**：finite-state automaton over citation grammar，追踪当前在生成 claim / doc_id / snippet 哪个状态，每步把候选 token 限制到语法一致且（snippet 状态下）与源文档逐词一致的集合；失败重试至多 3 次，每次温度 +0.5，全败则输出 "Cannot answer"。
   - **Posthoc span alignment**：先自由生成，若 snippet 与源文非逐词匹配（LLM 常产生 near-verbatim、差几个 token 的引文），在 doc_id 正确的前提下用 **word-level Jaccard similarity（阈值 0.7，经验最优）** 在源文档内找最相似片段重构引文。
2. **评测三轴**：Doc-F1（引用文档集合 vs gold 文档集合）；Snippet-F1（生成 span vs gold span 的 ROUGE-L / Jaccard token F1 / chrF++）；claim-citation faithfulness（MiniLM 相似度 + GPT-5.4 judge 1-5 分 + 人工 50 例）。
3. **数据**：ASQA / BioASQ / ExpertQA。ASQA、ExpertQA 只有文档级标注，作者用 GPT-5.4-mini 抽取 atomic evidence span 补标（在 BioASQ 上验证该流程：与 gold span 的 ROUGE-L、chrF++ >90%，token F1 85%），并人工清洗掉无显式证据的文档（丢弃 ExpertQA ~500 例、ASQA ~350 例）。
4. **Baseline**：Generate-then-Retrieve（先生成答案再用 BM25 + all-MiniLM-L6-v2 检索证据句——post-hoc 归因代表）与 ReClaim（两阶段约束解码，需分别训练，仅 span 级）。
5. **发现的两个系统性失败模式**：(i) primacy bias——BioASQ 5 篇上下文文档中 81.8% 的引用只指向前 2 篇（lost-in-the-middle）；(ii) yes/no 问题大量省略引用，使 prompt-based baseline 的下游分数虚高（一旦强制归因就消失）。

### 对我们的直接用途
- **方向 3 的 span-gap 量化论据**：Doc-F1 ≫ Snippet-F1 的普遍鸿沟（见下）直接支撑我们的立论——检索臂选对文档只解决一半问题，句子/span 级定位才是瓶颈；可引用其数字论证在 SciFact 上做句子级归因评测的必要性。
- 其 **posthoc Jaccard-0.7 span 对齐**是零训练、可直接移植的组件：我们 `src/rag_pipeline.py` 的 RAGResult 若要加 citation，可让 Granite 生成 near-verbatim 引文后用同法对齐回 retrieved chunk——比约束解码实现成本低且效果更好。
- primacy bias 的发现与我们 NIAH 的 needle-position 敏感性实验同源，可互相印证（citation 位置偏差 = 检索上下文利用不均）。
- Snippet-F1 评测代码逻辑（ROUGE-L/Jaccard/chrF++ 对 gold span）可用于我们的 extractive answer 与 SciFact rationale 对齐评分。

### 关键数字
- **Span gap 核心证据**（prompt-based, Qwen3-8B）：BioASQ Doc-F1 58.08 vs Snippet-F1 **6.18**；ExpertQA 56.42 vs 5.56；ASQA 33.87 vs 12.80——文档选对了但 span 几乎全错。
- Posthoc 对齐的增益：ASQA Snippet-F1 12.80 → **61.87**（Qwen3-8B），12.42 → 41.80（Gemma-3-12B）；Generate-then-Retrieve Doc-F1 可达 93.74（ASQA）但 Snippet-F1 仅 75.07/16.83/32.70（ASQA/BioASQ/ExpertQA），域内专业问题定位崩塌。
- Primacy bias：BioASQ 81.8% 引用集中于 5 篇文档的前 2 篇；ASQA 约 40% 的 posthoc/constrained 重试全部失败（上下文缺显式 verbatim 证据）。
- 下游任务：posthoc 设定 BioASQ 平均增益 +19.85 点（factoid 36.74→52.63）。

---

## 横向小结（对项目的映射）

| 论文 | 分类学位置 | 项目落点 |
|---|---|---|
| #9 Xetrieval | post-hoc（SAE on embeddings）+ intervention fidelity | 方向 2：Granite dense arm 的稀疏特征解释 + erase/keep-only fidelity 实验 |
| #10 CERA | inherently interpretable（attention 监督进训练） | 方向 3：SciFact rationale 监督/评测协议模板；ERASER 指标；hard-negative 思路对 NIAH counterfactual |
| #13 Pandian | 评价协议（MRC/MER） | 方向 2：SPLADE 词权重 vs dense occlusion 的统一 consistency 对比骨架 |
| #8 FullCite | 生成侧 attribution（3 种引用策略） | 方向 3：span-gap 定量论据 + posthoc Jaccard 对齐组件进 RAGPipeline |

统一叙事：#13 提供"怎么算分"，#9 提供 dense 臂"怎么产生解释"，#10 提供"gold rationale 怎么用作监督/评测"，#8 提供"生成端 span 定位是短板"的动机数字。
