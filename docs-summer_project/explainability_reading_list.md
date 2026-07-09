# Explainability Literature — Reading List for Survey Agent

## Project context *(updated 2026-07-05 to match week4 state)*

- **Project:** IBM_Granite_Project — Granite-powered retrieval + RAG system, now centred on a **NIAH stress test with counterfactual distractors** (a lone "Source-A" passage that is relevant but factually wrong).
- **Retrieval arms (three, not two):** BM25 (classical sparse) + **SPLADE** `naver/splade-cocondenser-ensembledistil` (learned sparse — *inherently interpretable term weights*) + `granite-embedding-english-r2` (dense). Fused by **tuned convex combination** (`eval/tune_alpha.py`); RRF lost to pure dense on every BEIR set (results-summary finding #4).
- **Rerankers:** Granite cross-encoder; LLM listwise (RankGPT-style); **CorroborationReranker** — cross-source answer voting (+ a parametric voter from the model's own knowledge) that demotes lone counterfactuals (`src/retrieval/corroboration.py`, design spec 2026-07-05). An Astute-RAG-style pipeline is already prototyped (`tests/test_astute_rag_pipeline.py`).
- **Datasets:** BEIR (SciFact / NFCorpus / FiQA) for nDCG-recall; NQ subset for RAG cover-EM/F1; custom NIAH task (needle + counterfactual distractors) scored by needle-found@k / MRR.
- **Chosen innovation direction:** **explainability**, framed as *evidence attribution* **and** *evidence corroboration* — verifiable sentence-level evidence, faithfulness of citations (parametric vs contextual), and robustness to conflicting/counterfactual evidence.
- **Evaluation stack already in place:** recall / precision / nDCG; needle-found@k / MRR; cover-EM / F1; paired significance tests (`eval/significance.py`).

## Task for the agent

For **each** paper below, produce a structured note capturing:

1. **Category** — where it sits in the taxonomy: `inherently-interpretable architecture` vs `post-hoc explanation`; and for attribution work: `in-generation` (citation produced during decoding) vs `post-hoc` (citation added after generation).
2. **Core method** — 2–3 sentences, no fluff.
3. **Evaluation** — metrics + datasets used (flag anything reusable with our nDCG / needle-found@k / cover-EM stack on SciFact-BEIR / NIAH).
4. **Relevance hook** — how it connects to *hybrid (BM25 + SPLADE + dense) retrieval × corroboration reranking × explainability* (our narrative), including NIAH counterfactual robustness.
5. **Limitation / failure mode** — especially any noted weakness of dense retrieval, LLM attribution, or parametric-vs-contextual conflict handling (we need these to expose Granite's edge cases per mentor feedback).

Then synthesize across all papers into: (a) a taxonomy table, (b) a gap analysis, (c) 2–3 candidate novel directions for our project.

**Priority legend:** `P0` = must-read core (read first) · `P1` = core · `P2` = supporting / deeper dive.

---

## Group A — Survey anchors (build the taxonomy from these first)

| # | Title | arXiv | URL | Year | Priority | Why relevant |
|---|-------|-------|-----|------|----------|--------------|
| 1 | Explainable Information Retrieval: A Survey | 2211.02405 | https://arxiv.org/abs/2211.02405 | 2022 | P1 | Foundational IR-explainability survey; source of the `inherently-interpretable vs post-hoc` taxonomy that structures the whole review. |
| 2 | Attribution, Citation, and Quotation: A Survey of Evidence-based Text Generation with LLMs | 2508.15396 | https://arxiv.org/abs/2508.15396 | 2025 | P0 | Most recent attribution survey; maps evaluation frameworks/benchmarks and the `in-generation vs post-hoc` split. Primary skeleton for our "explainability = evidence attribution" narrative. |
| 3 | Explainability of Text Processing and Retrieval Methods: A Survey | 2212.07126 | https://arxiv.org/abs/2212.07126 | 2022 | P2 | Broader coverage (counterfactuals, term-weighting); background breadth. |

## Group B — RAG attribution / faithfulness / citation evaluation (core narrative)

| # | Title | arXiv | URL | Year | Priority | Why relevant |
|---|-------|-------|-----|------|----------|--------------|
| 4 | ALCE: Enabling LLMs to Generate Text with Citations | 2305.14627 | https://arxiv.org/abs/2305.14627 | 2023 | P0 | Defines citation-quality recall/precision (does cited set entail the sentence; is each doc precisely cited). Directly bolts onto our existing recall/precision/nDCG metrics. |
| 5 | Correctness is not Faithfulness in RAG Attributions | 2412.18004 | https://arxiv.org/abs/2412.18004 | 2024 | P0 | Key concept: a "correct" citation (NLI-supported) can still be *unfaithful* (model used parametric memory, then post-rationalized a token-match citation). Essential for analyzing Granite's failure modes. |
| 6 | Model Internals-based Answer Attribution for Trustworthy RAG (MIRAGE) | 2406.13663 | https://arxiv.org/abs/2406.13663 | 2024 | P1 | Post-hoc attribution using model-internal signals; representative "open the black box" approach. |
| 7 | Generation-Time vs. Post-hoc Citation: A Holistic Evaluation of LLM Attribution | 2509.21557 | https://arxiv.org/abs/2509.21557 | 2025 | P1 | Systematic comparison of G-Cite vs P-Cite methods and datasets; good reference for our classification chapter. |
| 8 | Explicit Evidence Grounding via Structured Inline Citation Generation | 2606.07130 | https://arxiv.org/abs/2606.07130 | 2026 | P1 | Finds LLMs pick correct documents but struggle to locate the precise supporting *span* — exactly SciFact's rationale-sentence-selection task. |

## Group C — Retriever interpretability (explains our BM25 + Granite dense baselines)

| # | Title | arXiv | URL | Year | Priority | Why relevant |
|---|-------|-------|-----|------|----------|--------------|
| 9 | Xetrieval: Mechanistically Explaining Dense Retrieval | 2605.29507 | https://arxiv.org/abs/2605.29507 | 2026 | **P0** ↑ | Newest survey+method on explaining dense retrieval; contrasts interpretable architectures (SPLADE sparse weights, ColBERT token alignment) vs post-hoc (interaction attribution, surrogate, SAE features). **Upgraded to P0: SPLADE is now our implemented lexical arm — its sparse term weights are the inherently-interpretable half of our hybrid story.** |
| 10 | Beyond Topical Similarity: Contrastive Evidence Retrieval with Interpretable Attention Alignment in RAG | 2606.01482 | https://arxiv.org/abs/2606.01482 | 2026 | P1 | Interpretable attention alignment for evidence retrieval; strong fit for the hybrid + explainability combination. |
| 11 | Probing Ranking LLMs: A Mechanistic Analysis for IR | 2410.18527 | https://arxiv.org/abs/2410.18527 | 2024 | P2 | Mechanistic-interpretability lens on ranking; deeper dive for "why dense fails on certain queries." |
| 12 | How do LLMs Understand Relevance? A Mechanistic Interpretability Perspective | 2504.07898 | https://arxiv.org/abs/2504.07898 | 2025 | P2 | Companion to #11; relevance-formation internals. |
| 13 | Evaluating the Explainability of Neural Rankers | 2403.01981 | https://arxiv.org/abs/2403.01981 | 2024 | P2 | Evaluation angle — how to *quantify* explainability itself. |

## Group D — Scientific claim verification + evidence/rationale (SciFact — our dataset)

| # | Title | arXiv | URL | Year | Priority | Why relevant |
|---|-------|-------|-----|------|----------|--------------|
| 14 | Fact or Fiction: Verifying Scientific Claims (SciFact) | 2004.14974 | https://arxiv.org/abs/2004.14974 | 2020 | P0 | Origin of our dataset; defines the 3-stage task (abstract retrieval → rationale-sentence selection → stance) and the gold rationale sentences we evaluate evidence selection against. Must-cite. |
| 15 | SciFact-Open: Towards Open-domain Scientific Claim Verification | 2210.13777 | https://arxiv.org/abs/2210.13777 | 2022 | P1 | Scales corpus to 500K abstracts; systems trained on small corpora drop ≥15 F1 — a ready-made "expose the failure boundary" result. |
| 16 | DeepSciVerify: Verifying Scientific Claim–Citation Alignment via LLM-Driven Evidence Escalation | 2605.27710 | https://arxiv.org/abs/2605.27710 | 2026 | P1 | Two-stage pipeline: abstract-level verification with early-exit, escalating to passage-level only when evidence is insufficient. Aligns with the MVP/subset-first strategy from mentor feedback. |
| 17 | Step-by-step Fact Verification for Medical Claims with Explainable Reasoning | 2502.14765 | https://arxiv.org/abs/2502.14765 | 2025 | P1 | CoT-based explainable verification: generates clarification questions, retrieves from PubMed/Wikipedia. Representative "explainable verification" method. |
| 18 | Evaluating Evidence Attribution in Generated Fact-Checking Explanations (Xing, Baldwin, Lau) | 2406.12645 | https://arxiv.org/abs/2406.12645 | 2025 (NAACL) | P2 | Citation masking-and-recovery protocol for evaluating evidence attribution inside fact-checking explanations; LLM annotators correlate with humans. *(Link resolved 2026-07-05.)* |

## Group E — Knowledge conflict / corroboration (NEW — grounds the CorroborationReranker)

*Added 2026-07-05: the project's innovation is now corroboration reranking against counterfactual distractors; these papers are its direct intellectual ancestors and must anchor the related-work section.*

| # | Title | arXiv | URL | Year | Priority | Why relevant |
|---|-------|-------|-----|------|----------|--------------|
| 19 | Astute RAG: Overcoming Imperfect Retrieval Augmentation and Knowledge Conflicts for LLMs | 2410.07176 | https://arxiv.org/abs/2410.07176 | 2024 | P0 | Source-aware consolidation of internal (parametric) + external (retrieved) knowledge with reliability-based answer finalisation — the direct ancestor of our CorroborationReranker's parametric voter. Already prototyped in-repo (`tests/test_astute_rag_pipeline.py`). |
| 20 | ClashEval: Quantifying the tug-of-war between an LLM's internal prior and external evidence | 2404.10198 | https://arxiv.org/abs/2404.10198 | 2024 | P1 | Measures when models wrongly defer to perturbed/counterfactual retrieved evidence vs correctly holding their prior — exactly our Source-A failure mode; gives a calibration framing for weighting the parametric vote. |

---

## Suggested reading order

1. **First pass (taxonomy):** #2 → #1 → #14. Establishes framework + dataset grounding.
2. **Corroboration core (our innovation):** #19 → #5 → #20. Parametric-vs-contextual conflict, the reranker's intellectual ancestry.
3. **Attribution + evaluation:** #4 → #7 → #8.
4. **Retriever explainability (hybrid story):** #9 → #10, then #11/#12 if going deep.
5. **SciFact-specific:** #15 → #16 → #17.
6. **Fill-ins:** #3, #6, #13, #18.

## Five "must-read" cornerstones
`#4 ALCE` (metrics) · `#5 Correctness≠Faithfulness` (core concept) · `#14 SciFact` (dataset) · `#2 Attribution survey 2025` (skeleton) · `#19 Astute RAG` (corroboration ancestry).

## Output the agent should produce
- One structured note per paper (fields 1–5 above).
- A consolidated **taxonomy table** (rows = papers, columns = category / method / metrics / dataset / limitation).
- A **gap analysis** paragraph.
- **2–3 candidate novel directions** tying explainability to our hybrid (BM25 + SPLADE + Granite dense) + corroboration-reranking setup on SciFact-BEIR / NIAH.

> Note: IDs in the 2603–2606 range are recent (2026) preprints; verify each link resolves and pull the latest version. If any abstract page 404s, fall back to an arXiv title search.
