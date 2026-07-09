# Literature Review Briefing

---

## The narrative chain

Citations can be *correct* yet *unfaithful* — models post-rationalize by answering from parametric memory and attaching a matching document afterwards (#5). Existing knowledge-conflict handling operates only at the **generation stage** (#19, explicitly "post-retrieval" and orthogonal to retriever work), while systematic evaluation shows **retrieval quality is the primary driver of attribution quality** in both citation paradigms (#7). ClashEval (#20) quantifies the conflict — models override their own correct priors >60% of the time on perturbed evidence — and shows calibrated token-probability comparison can arbitrate it. **Gap:** no existing mechanism exploits cross-source consistency at the *reranking* stage. CorroborationReranker fills this gap; gains are measurable with clean retrieval metrics (needle-found@k, MRR) and downstream citation metrics from ALCE (#4).

---

## #5 — Correctness is not Faithfulness in RAG Attributions
*Wallat, Heuss, de Rijke, Anand · arXiv 2412.18004 · 2024*

- **Claim:** Citation **correctness** (cited doc supports the sentence) ≠ **faithfulness** (model genuinely relied on the doc). The gap is **post-rationalization**: answer from parametric memory first, attach a matching citation afterwards.
- **Key number:** up to **57% of citations post-rationalized** — *on adversarial test cases* (upper bound, not an in-the-wild rate).
- **Method:** intervention-based experiments (perturb/remove cited evidence, observe response), design borrowed from CoT-unfaithfulness work.
- **Relevance:** Theoretical backbone. Our `use_parametric` vote surfaces the parametric-vs-context tension *at retrieval time*; NIAH's self-constructed needle/counterfactual pairs make the intervention probe **zero-cost** (we plant the evidence ourselves).

## #19 — Astute RAG
*Wang et al. · Google Cloud AI Research · arXiv 2410.07176 · 2024*

- **Claim:** Imperfect retrieval is pervasive (~70% of retrieved passages lack the true answer); internal and external knowledge correct each other at comparable rates (47.4% vs 52.6% on conflict cases).
- **Method:** three generation-stage steps — adaptive internal-passage generation → iterative source-aware consolidation (group consistent / split conflicting / drop irrelevant) → confidence-based answer finalization. Only method matching No-RAG under worst-case (all-negative) retrieval.
- **Verified positioning:** operates strictly **post-retrieval**; explicitly orthogonal to retriever/reranking work. It uses cross-source confirmation to *pick answers*, never to *rank passages*.
- **Relevance:** Direct ancestor. We move the same corroboration signal one stage earlier, decoupling it from the generator and making gains measurable with pure retrieval metrics. Shared limitation: reliance on LLM capability; its confidence scores are self-reported, uncalibrated (→ #20).

## #7 — Generation-Time vs. Post-hoc Citation
*Saxena et al. · UMBC · NeurIPS 2025 Workshop · arXiv 2509.21557*

- **Claim:** Across both citation paradigms (G-Cite: cite while decoding; P-Cite: draft first, cite after), **retrieval augmentation is the primary driver of citation quality** (e.g., FEVER G-Cite correctness 27% → 77% zero-shot → RAG).
- **Trade-off:** stable coverage-vs-correctness tension — G-Cite precise but narrow and slow; P-Cite broad with moderate latency. Recommendation: P-Cite-first for high-stakes use; G-Cite for strict claim verification (fits SciFact, which is P-Cite-native).
- **Caveats:** single 8B model (LLaMa-3.1-8B) → cite relative trends, not absolute numbers; workshop short paper.
- **Relevance:** External justification for placing our innovation at the retrieval/reranking layer — upstream of the paradigm choice entirely.

## #20 — ClashEval
*Wu, Wu, Zou · Stanford · NeurIPS 2024 D&B · arXiv 2404.10198*

- **Claim:** On controlled evidence perturbations (1,294 questions, 6 domains), top LLMs override their own **correct** prior >60% of the time (context bias). Two-sided design (context-wrong/prior-right *and* the converse) prevents the trivial "ignore context" shortcut.
- **Two usable signals:** (1) larger perturbations are adopted less; (2) higher prior token probability → stronger resistance to context.
- **Fix:** **Calibrated Token Probability Correction** — compare prior vs contextual response confidence by *percentile within their own distributions* (raw probabilities are incomparable: contextual confidences are right-tailed, priors near-uniform). Result: GPT-4o accuracy 61.5% → 75.4%, context bias 30.4% → 10.7%.
- **Caveats:** error-enriched dataset (rates ≠ in-the-wild); single-document context in main experiments; requires logprob access.
- **Relevance:** Upgrade path for our parametric voter — from a binary vote to a **calibrated, confidence-weighted vote** (change localized to `corroboration_scores` weighting). Self-hosted Granite gives us the required logprob access.

## #4 — ALCE: Enabling LLMs to Generate Text with Citations
*Gao, Yen, Yu, Chen · Princeton · EMNLP 2023 · arXiv 2305.14627*

- **Contribution:** first automatic, reproducible benchmark for citation quality (ASQA / QAMPARI / ELI5). Three joint dimensions — fluency, correctness, citation quality — explicitly designed to block shortcut solutions.
- **Core algorithm (sentence-level, NLI judge φ = TRUE / T5-11B):**
  - **Citation recall** = 1 iff concat(cited passages) entails the sentence.
  - **Citation precision**: a citation is *irrelevant* iff it alone does not entail the sentence AND removing it leaves the rest still entailing. Known blind spot: partial support undetected → precision underestimated.
- **Key findings:** retrieval recall upper-bounds performance; LLMs underuse correct answers even when present in context; more passages plateau quickly (ChatGPT: correctness at top-1, citations at top-3); ~50% of ELI5 generations not fully supported.
- **Relevance:** Our downstream metric source. Fully open-source (incl. the NLI judge on HuggingFace) — plugs directly behind our existing recall/precision/nDCG stack to test the causal chain: *corroboration reranking ↑ needle-found@k → citation recall ↑*.
