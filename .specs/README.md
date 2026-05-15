# .specs - Mapa de Navegacao

Esta pasta concentra o planejamento executavel de **US4 V6 Apple Edition**.

Ela precisa refletir duas verdades ao mesmo tempo:

1. o produto final planejado eh um runtime C++/MLX/Metal para Apple Silicon;
2. o estado atual do repo ainda e o bootstrap do starter, antes do scaffold do runtime.

Sempre que houver tensao entre "estado atual do repositorio" e "estado planejado do produto", os docs precisam deixar isso explicito.

## Ordem de leitura

1. `product/VISION.md`
2. `product/PERSONAS.md`
3. `product/DOMAIN.md`
4. `architecture/DESIGN.md`
5. `architecture/PATTERNS.md`
6. `architecture/ADR-*.md`
7. `workflow/WORKFLOW.md`
8. `workflow/CONTRIBUTING.md`
9. `workflow/RELEASE.md`
10. `sprints/BACKLOG.md`
11. `sprints/TIMELINE.md`
12. `sprints/sprint-XX/SPRINT.md`
13. `sprints/sprint-XX/*.task.md`

## Regras desta pasta

- Produto e arquitetura devem descrever o **runtime Apple real**, nao um template generico web.
- Workflow e release devem distinguir claramente o que **ja existe** do que eh **planejado para sprints futuras**.
- O sprint atual precisa ter task files suficientes para execucao sem adivinhacao.
- Placeholder de template so e aceitavel em arquivos de template.

## Estado esperado por area

| Area | Deve refletir |
|---|---|
| `product/` | posicionamento, usuarios, capacidades e limites do runtime Apple |
| `architecture/` | boundaries, contratos e padroes do runtime C++/MLX/Metal |
| `workflow/` | processo real do repo hoje + transicao para o repo de runtime |
| `sprints/` | roadmap, dependencias e corte executavel do sprint atual |
