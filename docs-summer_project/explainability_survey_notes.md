# Explainability 文献综述笔记

> 依据 `explainability_reading_list.md`（2026-07-05 调整版）产出。项目锚点：**hybrid（BM25 + SPLADE + Granite dense，convex fusion）× CorroborationReranker（反事实干扰项）× 证据归因**。
> 笔记字段：Category / Core method / Evaluation / Relevance hook / Limitation。
> 阅读深度说明：2026 年与截止后论文基于 arXiv 摘要页联网核实（2026-07-05 抓取）；其余基于已有文献知识。带 ⚠ 的条目建议精读原文后补充。

---

## Group A — Survey anchors

### #1 Explainable Information Retrieval: A Survey (arXiv 2211.02405, 2022)

1. **Category**：综述（奠基性 EXIR survey）。提出本清单沿用的 `inherently-interpretable（by-design）vs post-hoc` 主轴，并按解释粒度（pointwise / pairwise / listwise）与解释形式（feature attribution、free-text、adversarial/counterfactual）细分。
2. **Core method**：对可解释 IR 方法做系统分类：一侧是结构上透明的模型（稀疏词权重、显式词项交互），另一侧是对黑盒 ranker 的事后解释（LIME/SHAP 类局部代理、axiomatic 诊断、探针）。
3. **Evaluation**：综述层面无新指标；整理了 fidelity（解释是否忠实于模型）与 plausibility（人类是否觉得合理）的区分——这一对概念可直接复用到我们的评价章节。
4. **Relevance hook**：为「BM25/SPLADE 天生透明 vs Granite dense 不透明」提供分类学词汇；我们的 hybrid 天然横跨该 survey 的两大类，convex fusion 权重 α 本身可作为 query 级「哪个 arm 在起作用」的解释信号。
5. **Limitation / failure mode**：成文于 LLM-RAG 之前，不覆盖生成侧 citation/attribution；对 dense retriever 的解释手段当时仅有代理模型与注意力可视化，fidelity 存疑。

### #2 Attribution, Citation, and Quotation: A Survey of Evidence-based Text Generation with LLMs (arXiv 2508.15396, 2025) — P0 骨架

1. **Category**：综述（证据式生成）。统一 attribution / citation / quotation 术语，覆盖 `in-generation vs post-hoc` 的引用产生时机划分。
2. **Core method**：系统分析 134 篇论文，提出统一 taxonomy，并盘点 **300 个评价指标、归入 7 个维度**（正确性、忠实性、覆盖度等）。
3. **Evaluation**：指标地图本身即贡献；明确指出该领域「术语不一致、评价各自为政、缺统一 benchmark」。
4. **Relevance hook**：作为我们「explainability = evidence attribution + corroboration」叙事的骨架。它诊断的「缺统一 benchmark」正是空档——我们的 NIAH（反事实）+ SciFact（gold rationale）组合可定位为一个检索侧 + 生成侧联动的评测。
5. **Limitation / failure mode**：survey 自身指出评价维度间常互相冲突（如 coverage vs precision），单一指标结论不可信——我们汇报 corroboration 效果时必须多维（needle-found@k + citation P/R + faithfulness）。

### #3 Explainability of Text Processing and Retrieval Methods: A Survey (arXiv 2212.07126, 2022)

1. **Category**：综述（更宽：embedding、transformer、IR 模型的可解释性；含 counterfactual、term-weighting 解释）。
2. **Core method**：按被解释对象（词向量 → 神经 ranker）组织事后解释技术：attribution、probing、counterfactual 生成。
3. **Evaluation**：无统一指标；罗列各方法自带的评价。
4. **Relevance hook**：背景纵深。term-weighting 解释一节与 BM25/SPLADE 的词权重可解释性直接相通，可引用来论证「学习型稀疏 = 保留词项级可解释同时获得语义匹配」。
5. **Limitation / failure mode**：同样 pre-RAG；对「解释是否忠实」的评估方法着墨少。

---

## Group B — RAG attribution / faithfulness / citation evaluation

### #4 ALCE: Enabling LLMs to Generate Text with Citations (arXiv 2305.14627, 2023) — P0 指标源

1. **Category**：benchmark + 评价框架；引用属 **in-generation**（prompt 内给检索段落、生成时带 `[k]` 引用标记）。
2. **Core method**：ALCE 基准（ASQA / QAMPARI / ELI5 三数据集）+ 自动评价：用 NLI 模型判断「被引集合是否蕴含该句」（citation recall）与「每条引用是否必要」（citation precision），另测 fluency、correctness。
3. **Evaluation**：**citation recall / precision 可直接接到我们现有 recall/precision/nDCG 栈之后**：对 NQ-subset RAG 输出或 SciFact 声明验证输出逐句打分；NLI 判定器可用现成模型（TRUE / T5-NLI）。
4. **Relevance hook**:给 corroboration reranker 的下游收益一个生成侧指标——假设：corroboration 提升第一阶段 needle-found@k → 下游 citation recall 同步上升，形成「检索侧干预 → 归因质量」的因果链叙事。
5. **Limitation / failure mode**：NLI 判定本身有误差；引用质量强烈受检索质量牵制（与 #7 的发现一致）；**只测 correctness，不测 faithfulness**（#5 的批评正落在这里）。

### #5 Correctness is not Faithfulness in RAG Attributions (arXiv 2412.18004, Wallat, Heuss, de Rijke, Anand, 2024) — P0 核心概念

1. **Category**：分析/批判性实证；针对 post-hoc 与 in-generation 引用的 **faithfulness 评价**。
2. **Core method**：区分 citation **correctness**（被引文档确实支持该句）与 **faithfulness**（模型生成时真的依赖了被引文档）。实验揭示「post-rationalization」：模型先用参数化记忆作答，再挑一篇字面吻合的文档贴上引用。
3. **Evaluation**：干预式实验设计；关键数字：**高达 57% 的引用不忠实**（即使其中很多在 NLI 意义上「正确」）。
4. **Relevance hook**：这是 CorroborationReranker 的理论靠山——我们把 parametric answer 显式拆成一张独立的票（`use_parametric`），等于把「参数化记忆 vs 上下文证据」的张力在检索阶段就摆上台面。而且 **NIAH 反事实任务给了可测的 post-rationalization 探针**：needle 与 counterfactual 答案互斥，若模型答案与 counterfactual 一致却引用 needle（或反之），即被当场捉住。
5. **Limitation / failure mode**：faithfulness 只能靠干预（删证据、换证据）间接测量，成本高；对我们的启示是 NIAH 的受控构造（我们自己埋 needle 和 counterfactual）恰好把干预成本降为零——这是自建任务相对公开 benchmark 的独特优势。

### #6 MIRAGE: Model Internals-based Answer Attribution for Trustworthy RAG (arXiv 2406.13663, 2024)

1. **Category**：**post-hoc** attribution，基于模型内部信号（打开黑盒）。
2. **Core method**：先用对比法检测答案中哪些 token 是 context-sensitive（受检索段落影响而非参数记忆），再用 saliency 把这些 token 归因到具体的检索文档，产出细粒度引用。
3. **Evaluation**：与人工/NLI 归因对比一致性（多语 QA、长答案 QA 场景）；无需外部 NLI 判定器。
4. **Relevance hook**：**需要白盒权重——而我们的 Granite 是自托管的**，API 模型做不了这件事，这是项目的差异化条件。可用 MIRAGE 式内部信号验证 corroboration 的机制假设：被投票支持的段落是否真的在生成时被更多使用。
5. **Limitation / failure mode**：saliency 方法自身的可靠性争议；粒度受 tokenizer 影响；计算开销随上下文长度增长（对 NIAH 长上下文场景要注意成本）。

### #7 Generation-Time vs. Post-hoc Citation: A Holistic Evaluation of LLM Attribution (arXiv 2509.21557, Saxena et al., 2025)

1. **Category**：系统性对比评测（G-Cite = in-generation vs P-Cite = post-hoc 的横评）。
2. **Core method**：在 zero-shot 到高级 RAG 的多种配置、四个归因数据集上对比两种范式；核心结论：**检索质量是两种范式共同的性能瓶颈**；存在稳定的 coverage-vs-correctness 权衡（G-Cite 精确但覆盖窄且慢，P-Cite 覆盖广、延迟适中）。
3. **Evaluation**：四个 attribution 数据集，多维 citation 质量指标（可与 #4 的 P/R 对齐）。
4. **Relevance hook**：「检索质量决定归因上限」直接支撑我们把创新做在**检索/重排阶段**（convex hybrid 调优 + corroboration）而非生成阶段的立项逻辑；其建议「严格声明验证场景用 G-Cite」适配 SciFact。
5. **Limitation / failure mode**：摘要未列明确 limitation；G-Cite 的延迟与我们 LLM 重排的 per-query 成本问题同构（`top_n`+1 次调用/query），可类比讨论。

### #8 FullCite: Explicit Evidence Grounding via Structured Inline Citation Generation (arXiv 2606.07130, Yeginbergen et al., 2026)

1. **Category**：混合——in-generation（prompt 生成 + citation-grammar 约束解码）与 post-hoc（span 对齐）三策略并测。
2. **Core method**：生成结构化行内引用，把 claim 同时连到源文档与文档内支持证据 span；对比 prompt、约束解码、事后 span 对齐三种实现。
3. **Evaluation**：ASQA / BioASQ / ExpertQA；测 document-level correctness、evidence-span 识别、claim-citation faithfulness——BioASQ 与我们 NFCorpus/SciFact 的生物医学域重合。
4. **Relevance hook**：核心发现「**LLM 会选对文档，但定位不到确切支持 span**」就是 SciFact 第二阶段（rationale-sentence selection）的痛点。我们手上有 gold rationale sentences，可以零标注成本量化这个 gap。
5. **Limitation / failure mode**：span 级归因仍未解决——这是我们候选方向 3 的直接依据。

---

## Group C — Retriever interpretability

### #9 Xetrieval: Mechanistically Explaining Dense Retrieval (arXiv 2605.29507, Cai et al., 2026) — 升级 P0

1. **Category**：**post-hoc 机制式解释**（embedding 级分解，SAE 风格）。⚠ 注意：抓取确认它主要是**方法**而非清单原先描述的「survey+method」——related work 里有分类学讨论，但引用时应作为方法论文。
2. **Core method**：用轻量「reasoning internalizer」给 dense embedding 注入推理信息，再把 embedding 分解为**带自然语言描述的可解释稀疏特征**，实现 feature 级检索解释，且无需昂贵的生成式解释。
3. **Evaluation**：多检索器多基准；评特征连贯性、pair 级干预效应、task 级 feature steering。
4. **Relevance hook**：补全我们 hybrid 的「对称可解释性」：SPLADE arm 天生输出稀疏词权重（inherently interpretable），Xetrieval 给 dense arm 提供对应的事后稀疏特征分解——两臂都能回答「为什么召回/漏掉 needle」。可用于诊断 granite dense 在 NIAH 上漏检 needle 的 query。
5. **Limitation / failure mode**：可解释特征的连贯性评价尚不成熟；分解出的特征是否忠实于原检索打分（fidelity）仍需干预验证。

### #10 CERA: Contrastive Evidence Retrieval with Interpretable Attention Alignment in RAG (arXiv 2606.01482, Vargas et al., 2026)

1. **Category**：**inherently-interpretable（训练进模型）**——用监督把可解释性内建到 retriever。
2. **Core method**：对 dense retriever 做双目标微调：triplet 对比学习 + 可解释注意力对齐（用 POS 加权的人工 rationale 掩码监督 CLS-to-token attention）；配合基于主观性的 hard-negative 选择。
3. **Evaluation**：临床试验报告语料；对比 Contriever 与 hard-negative 基线；检索效果指标 + faithfulness 度量。
4. **Relevance hook**：两条线都接得上——(a) SciFact 的 gold rationale sentences 正是它需要的监督信号，可想象一个「rationale-aligned granite-embedding」微调；(b) 它的 hard-negative 选择与我们 NIAH 的 counterfactual distractor 构造（`src/niah/counterfactual.py`）是同一思想的训练侧/评测侧两面。
5. **Limitation / failure mode**：域特定（临床试验），泛化未知；依赖人工 rationale 标注（SciFact 恰好有，别的集没有）。

### #11 Probing Ranking LLMs: A Mechanistic Analysis for IR (arXiv 2410.18527, 2024)

1. **Category**：post-hoc 机制式（逐层 probing）。
2. **Core method**：对 ranking LLM 的中间表示逐层训练探针，追踪「相关性」信号在网络中何处形成、如何演化。
3. **Evaluation**：标准 ranking 数据上的探针精度随层数变化曲线。
4. **Relevance hook**：深挖层备选：解释我们 cross-encoder / LLM listwise reranker 的行为；优先级低于 #9/#10。
5. **Limitation / failure mode**：probing 是相关性证据而非因果证据；结论依赖具体模型。

### #12 How do LLMs Understand Relevance? A Mechanistic Interpretability Perspective (arXiv 2504.07898, Liu, Mao, Wen, 2025)

1. **Category**：post-hoc 机制式（activation patching，因果性强于 #11 的 probing）。
2. **Core method**：用 activation patching 定位相关性判断的组件级通路，发现多阶段流水线：**早层抽取 query/doc 信息 → 中层按指令加工相关性 → 晚层特定 attention head 输出格式化判断**。
3. **Evaluation**：pointwise/pairwise 相关性判断任务上的组件干预分析。
4. **Relevance hook**：与 #11 组成机制式解释的配对参考；若要解释 `LLMListwiseReranker` 为何产生某个排列（或为何被 counterfactual 骗），这是方法论出处。
5. **Limitation / failure mode**：范围限于现成模型的分析，跨架构/跨规模泛化未验证。

### #13 Evaluating the Explainability of Neural Rankers (arXiv 2403.01981, 2024)

1. **Category**：元评价——如何**量化**解释本身的质量。
2. **Core method**：为 ranker 解释建立评价协议（fidelity：解释是否反映模型真实依据；completeness：解释是否足以复现判断），横向比较解释方法。
3. **Evaluation**：在标准 ranking 模型/数据上对多种解释方法打分。
4. **Relevance hook**：如果我们声称「SPLADE 词权重解释优于 dense 的事后解释」，需要用这类 fidelity 协议来证明，而不是只展示可视化——评审一定会问。
5. **Limitation / failure mode**：解释质量指标本身是代理指标；不同协议间结论可能不一致。

---

## Group D — SciFact 声明验证与 rationale

### #14 Fact or Fiction: Verifying Scientific Claims — SciFact (arXiv 2004.14974, Wadden et al., 2020) — P0 数据集

1. **Category**：数据集/任务定义；gold rationale sentences = **extractive 解释的 ground truth**。
2. **Core method**：1,409 条专家撰写的科学声明配 5,183 篇摘要；三阶段任务：abstract retrieval → rationale-sentence selection → stance（SUPPORT/REFUTE/NEI）；VeriSci 基线。
3. **Evaluation**：abstract-level 与 sentence-level 的 F1（后者要求选出的 rationale 句集正确）。**我们目前经 BEIR 只用了第一阶段（nDCG/recall）；第二、三阶段的标注闲置着。**
4. **Relevance hook**：gold rationale 是全项目最便宜的 explainability 评测资产——句子级证据归因、#10 式注意力对齐监督、#8 的 span-gap 量化，全都零新增标注。
5. **Limitation / failure mode**：语料小（5K 摘要），检索偏简单（→ #15 补刀）；系统可在 rationale 选错的情况下 stance 蒙对——「答案对但证据错」正是 faithfulness 问题在该数据集上的具象。

### #15 SciFact-Open (arXiv 2210.13777, 2022)

1. **Category**：数据集扩展（开放域压力测试）。
2. **Core method**：把验证语料扩到 500K 摘要，用系统池化（pooling)收集新证据对；在小语料上训练的系统 **F1 掉 ≥15 点**。
3. **Evaluation**：开放域下的 claim verification F1 + 池化标注。
4. **Relevance hook**：与我们 NIAH 的 `--max-docs` 规模扫描是同一实验设计（性能随语料规模的衰减曲线）；可互为印证：「检索失败边界随规模扩大而恶化」在公开数据集与自建任务上双重出现。
5. **Limitation / failure mode**：池化标注不完备（未被任何系统召回的证据无标注）→ 指标是下界。

### #16 DeepSciVerify (arXiv 2605.27710, Sadeghi et al., 2026)

1. **Category**：pipeline 方法；解释性体现在**分级证据升级**（先摘要级、置信不足才升级到全文段落级）。
2. **Core method**：两阶段验证：摘要级快速判定（保守模型）+ 置信不足时选择性升级到 passage 级检索与分析（更果断的模型）。
3. **Evaluation**：SCitance 基准，Micro-F1 = 86.7（比 abstract-only 强基线 +4.5），**67% 实例无需全文检索即解决**。
4. **Relevance hook**：其 cost-bounded escalation 与我们 corroboration 设计规格中**被推迟的「cost-bounded entailment tie-breaker」**（spec 2026-07-05）是同一模式——只在票数打平/冲突时才花贵的调用。做 tie-breaker PR 时应引用它。
5. **Limitation / failure mode**：限于科学验证场景；两个 LLM 角色（保守/果断）的搭配需调参，升级阈值即新的超参。

### #17 Step-by-step Fact Verification for Medical Claims with Explainable Reasoning (arXiv 2502.14765, Vladika et al., 2025)

1. **Category**：**in-generation** 可解释推理（迭代式 QA，非三段式流水线）。
2. **Core method**:迭代多轮：LLM 不断生成澄清问题并检索作答（PubMed/Wikipedia），直到信息足以裁决；辅以 logic predicates 的结构化推理，推理链即解释。
3. **Evaluation**：三个医学事实核查数据集；多配置（不同 LLM、外部搜索开关、逻辑谓词开关）。
4. **Relevance hook**：其「一问一检索」与我们 corroboration 的「一段一抽答案再投票」互为生成侧/检索侧对偶；医学域与 NFCorpus/SciFact 契合。可作为「explainable verification」代表方法在 related work 中与我们对照。
5. **Limitation / failure mode**：多轮 LLM 调用成本高（我们 corroboration 已把成本压到 top_n+1 次，这是可比优势）；作者自承对真实世界声明的探索不足。

### #18 Evaluating Evidence Attribution in Generated Fact-Checking Explanations (arXiv 2406.12645, Xing, Baldwin, Lau, NAACL 2025)

1. **Category**：评价方法学（对生成解释中归因质量的 post-hoc 评测协议）。
2. **Core method**：**citation masking and recovery** 协议——遮蔽解释中的引用，看能否从证据恢复，以此度量归因质量；用人工与 LLM 双注释员实施。
3. **Evaluation**：LLM 注释与人工注释显著相关（→ 可用 LLM 做廉价评测员）；最强 LLM 生成的解释仍有归因不准；人工筛选的证据能显著改善解释质量。
4. **Relevance hook**：两点直接可用：(a) masking-and-recovery 可评我们 RAG 输出的解释而无需新标注；(b) 「更好的证据 → 更好的解释」为「检索侧 corroboration 改善下游解释质量」的假设提供了文献支撑。
5. **Limitation / failure mode**：LLM 注释员自身偏差；协议测的是 attribution 的可恢复性，非 faithfulness（与 #5 互补而不重叠）。

---

## Group E — 知识冲突 / corroboration（新增组）

### #19 Astute RAG: Overcoming Imperfect Retrieval Augmentation and Knowledge Conflicts for LLMs (arXiv 2410.07176, Wang et al., 2024) — P0 血缘

1. **Category**：生成时（in-generation）知识整合方法；处理 internal（parametric）vs external（retrieved）冲突。
2. **Core method**：三步：自适应地从模型内部知识生成补充段落 → **source-aware consolidation**（把内外部段落按一致/冲突分组归并）→ 按各组可靠性对比敲定最终答案。
3. **Evaluation**：NQ、TriviaQA、BioASQ、PopQA 等 QA 集（Gemini/Claude 后端）；在检索质量差、乃至全部检索段落被污染的最坏情形下保持稳健。
4. **Relevance hook**：CorroborationReranker 的直系祖先，且仓库已有原型（`tests/test_astute_rag_pipeline.py`）。**关键差异要在论文里讲清**：Astute RAG 在生成阶段做段落归并，效果只能用答案指标衡量；我们把同一「跨源一致性」信号前移到**重排阶段**，因此可以用纯检索指标（needle-found@k / MRR）干净地归因收益，且与任意下游生成器解耦。
5. **Limitation / failure mode**：依赖 LLM 自身检测冲突的能力；parametric 知识本身可能是错的（无校准）——我们的 parametric voter 目前也是一票制，同样继承此弱点（→ #20 给出改进方向）。

### #20 ClashEval: Quantifying the tug-of-war between an LLM's internal prior and external evidence (arXiv 2404.10198, 2024)

1. **Category**：benchmark/分析（知识冲突的量化测量）。
2. **Core method**：构造带受控扰动的检索证据（药物剂量、日期、人名等被改错），测量模型在「坚持先验」与「服从错误上下文」间的行为；发现模型**过半情况会被错误上下文带偏**，且用 token 概率类信号可部分仲裁该冲突。
3. **Evaluation**：六个域的扰动数据集；指标 = 先验保持率 / 被带偏率随扰动强度的曲线。
4. **Relevance hook**：ClashEval 的「被扰动证据」与我们 NIAH 的 Source-A counterfactual 是同一失败模式的两种构造；其「token 概率可仲裁冲突」的发现指出我们 parametric voter 的升级路径——**从二值一票升级为按模型置信度加权的校准票**。
5. **Limitation / failure mode**：扰动是合成的，与自然发生的错误分布有偏差（我们的 counterfactual mining 从真实语料挖掘，可作为对照论据）；域覆盖有限。

---

## 综合产出

### (a) Taxonomy 表

| # | 论文 | 大类 | in-gen / post-hoc | 核心方法 | 指标 / 数据集 | 关键局限 |
|---|------|------|-------------------|----------|----------------|----------|
| 1 | EXIR Survey '22 | 综述 | — | by-design vs post-hoc 分类学 | fidelity/plausibility 概念 | pre-RAG |
| 2 | Attribution Survey '25 | 综述 | 两者均覆盖 | 134 篇统一 taxonomy；300 指标/7 维 | 指标地图 | 领域碎片化本身 |
| 3 | Text/IR XAI Survey '22 | 综述 | post-hoc 为主 | attribution/probing/counterfactual 盘点 | — | pre-RAG |
| 4 | ALCE | benchmark/评价 | in-generation | NLI 判 citation recall/precision | ASQA/QAMPARI/ELI5 | 只测 correctness 不测 faithfulness |
| 5 | Correctness≠Faithfulness | 分析 | 评价两者 | 干预实验揭示 post-rationalization | ≤57% 引用不忠实 | faithfulness 测量成本高 |
| 6 | MIRAGE | post-hoc 方法 | post-hoc | 模型内部信号（context-sensitivity+saliency）归因 | 与人工/NLI 一致性 | 需白盒；saliency 可靠性 |
| 7 | G-Cite vs P-Cite | 评价 | 两者横评 | 四数据集系统对比 | citation 质量多维 | coverage-correctness 权衡 |
| 8 | FullCite | 方法+评价 | 混合三策略 | 结构化行内引用（grammar 约束解码等） | ASQA/BioASQ/ExpertQA | span 级定位不行 |
| 9 | Xetrieval | post-hoc 机制式 | post-hoc | embedding 分解为可解释稀疏特征 | 特征连贯性/干预/steering | 特征 fidelity 待验证 |
| 10 | CERA | by-design | — | rationale 监督的注意力对齐微调 | 临床试验语料+faithfulness | 域特定；需 rationale 标注 |
| 11 | Probing Ranking LLMs | post-hoc 机制式 | post-hoc | 逐层 probing 相关性形成 | 探针精度曲线 | 相关非因果 |
| 12 | LLM Relevance MI | post-hoc 机制式 | post-hoc | activation patching 三阶段通路 | 组件干预 | 泛化未验证 |
| 13 | Eval Explainability of Rankers | 元评价 | — | 解释质量协议（fidelity 等） | 解释方法横评 | 代理指标 |
| 14 | SciFact | 数据集 | — | 三阶段验证任务+gold rationale | abstract/sentence F1 | 语料小；证据错答案对 |
| 15 | SciFact-Open | 数据集 | — | 500K 开放域扩展 | F1 掉 ≥15 | 池化标注不完备 |
| 16 | DeepSciVerify | pipeline | — | 摘要级早退+段落级升级 | SCitance Micro-F1 86.7 | 升级阈值超参 |
| 17 | Step-by-step Med Verify | 方法 | in-generation | 迭代 QA+逻辑谓词，推理链即解释 | 3 个医学核查集 | 多轮成本高 |
| 18 | Evidence Attr in Explanations | 评价方法学 | post-hoc 评测 | citation masking-and-recovery | 人工/LLM 双注释 | LLM 注释偏差 |
| 19 | Astute RAG | 方法 | in-generation | source-aware 内外知识归并 | NQ/TriviaQA/BioASQ 等 | 无校准的冲突检测 |
| 20 | ClashEval | benchmark | — | 受控扰动测先验 vs 上下文 | 六域扰动集 | 合成扰动有偏 |

### (b) Gap analysis

四个空档，均落在我们已有代码的延长线上。**其一（主空档）：知识冲突处理与检索重排之间没有桥。** 冲突类工作（#19 Astute RAG、#20 ClashEval）全部作用在生成阶段或只做测量，归因类工作（#4–#8）评的是生成后的引用质量，而检索/重排阶段没有任何机制利用跨源一致性信号——CorroborationReranker 恰好填这个位置，且因为作用在检索阶段，收益可以用 needle-found@k 这类纯检索指标干净度量，不与生成器纠缠。**其二：faithfulness 只被测量、未被机制性预防**（#5 证明 post-rationalization 普遍存在，但没给检索侧防御手段；我们的 parametric-vote 分离 + NIAH 受控反事实提供了「零成本干预探针」）。**其三：span/句子级归因是公认短板**（#8、#14）——LLM 选对文档却定位不到支持句，而 SciFact 的 gold rationale 标注在我们的 BEIR 用法里完全闲置。**其四：hybrid 两臂的可解释性不对称从未被下游验证**——SPLADE 词权重天生可解释（#9 语境），但没有工作检验「可解释的检索臂是否带来更可归因的生成」。

### (c) 候选创新方向（2–3 个）

1. **Corroboration-aware reranking 作为检索侧 faithfulness 防御**（主打，代码已就位）。叙事链：#5 证明引用会被 post-rationalize → #19/#20 显示冲突处理停留在生成阶段 → 我们把跨源答案投票前移到重排，用 NIAH 反事实任务测 needle-found@k/MRR（检索侧），再接 #4 的 citation P/R 与 #18 的 masking 协议（生成侧），形成两级证据。增量实验（来自 #20）：把二值 parametric 票升级为 token-probability 加权的校准票——改动只在 `corroboration_scores` 的投票权重，一个 PR 的量。
2. **Hybrid 两臂的对称可解释性 + α 的 query 级解释**。SPLADE arm 输出稀疏词权重（inherent，#9 语境），dense arm 用 MIRAGE/Xetrieval 式事后归因；对每个 query 解释「convex fusion 为何这一票由 lexical/dense 赢」，用已有的 per-query CSV（`results/*_per_query.csv`）+ `tune_alpha` 曲线做定量支撑，并按 #13 的 fidelity 协议验证解释质量。产出是「可解释的 hybrid 检索」一章，不需要新模型。
3. **零新增标注的句子级证据归因（SciFact rationale 复活）**。把 chunk→doc 的 max-pool（CONTRACT 3）细化到句子级打分，对照 gold rationale sentences 测 sentence-level recall/precision；同时检验 corroboration 的答案抽取 span 是否落在 gold rationale 内——直接回应 #8 指出的 span-gap。风险最低，但单独成篇的新颖性弱，适合作为方向 1 的支撑实验。

**推荐组合**：方向 1 为主干（与已实现的 CorroborationReranker、NIAH 任务、λ-sweep 完全对齐），方向 3 作为其 SciFact 侧的证据补强；方向 2 视时间作为可解释性章节的加分项。
