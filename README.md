# 🧾 Sistema de Faturação e Gestão de Stock em C
### iaed26 — ist1119719 | IST 2025/2026

![Build Status](https://github.com/eduardovianga23-j/Sistema-de-Fatura-o-e-Gest-o-de-Stock-em-C-Projeto-IAED---IST-2025-2026/actions/workflows/ci.yml/badge.svg)
![Language](https://img.shields.io/badge/language-C-blue.svg)
![GCC](https://img.shields.io/badge/gcc-12.3-orange.svg)
![Docker](https://img.shields.io/badge/docker-ready-2496ED?logo=docker)
![IST](https://img.shields.io/badge/IST-IAED%202025%2F2026-009900)

> Projeto desenvolvido no âmbito da cadeira de **IAED 2025/2026** no **Instituto Superior Técnico**.
> Sistema completo de faturação com gestão dinâmica de memória, validação de códigos EAN,
> processamento de comandos e emissão de faturas — desenvolvido em **C puro**.

---

## 📚 Sobre o Projeto

Este repositório pertence a **ist1119719** e destina-se ao projeto de iaed26.

O enunciado do projecto está disponível em [enunciado.md](enunciado.md).

Os alunos devem submeter aqui a sua solução para o project que será avaliada automaticamente.

O resultado da avaliação ficará disponível no [README de feedback](https://gitlab.rnl.tecnico.ulisboa.pt/iaed/iaed26/feedback/project/ist1119719/-/tree/master/README.md) após cada submissão.

O desempenho global pode ser consultado no [dashboard do projecto](https://gitlab.rnl.tecnico.ulisboa.pt/iaed/iaed26/iaed26/-/tree/master/dashboard/projects/project.md).

Informações detalhadas sobre depuração estão disponíveis em [debugging.md](debugging.md).

Outras guidelines podem ser encontradas em [guidelines.md](guidelines.md).

---

## ⚠️ Notas Importantes (IST)

- **Os alunos têm de esperar 15 minutos entre submissões.** Caso contrário a submissão não será avaliada.
- **Os alunos não podem alterar o ficheiro `.gitlab-ci.yml`** presente no repositório. A alteração deste ficheiro fará com que o aluno fique sem acesso ao repositório, não existirão excepções, e o aluno será avaliado com 0 valores.

---

## 📁 Estrutura do Projeto

```
.
├── proj.c              # Ponto de entrada — ciclo principal de comandos
├── commands.c/h        # Processamento de todos os comandos (p, a, f, l, d, r, c, u)
├── product.c/h         # Gestão de produtos
├── basket.c/h          # Cesto de compras
├── invoice.c/h         # Emissão e gestão de faturas
├── structs.h           # Definição de todas as estruturas de dados
├── utils.c/h           # Funções auxiliares (EAN, NIF, IVA, parsing)
├── Dockerfile          # Container Docker para compilar e correr o projeto
└── .github/
    └── workflows/
        └── ci.yml      # Pipeline CI/CD com GitHub Actions
```

---

## ⚙️ Compilação

```bash
gcc -O3 -Wall -Wextra -Werror -Wno-unused-result -o proj *.c
```

O sistema de avaliação automática usa **gcc versão 12.3.0**.

---

## 🧪 Testes Públicos

Após compilar, para correr os testes públicos:

```bash
# Descompactar os testes
unzip public-tests.zip
cd public-tests

# Correr todos os testes
make
```

Para correr um teste individualmente:

```bash
./proj < tests/teste01.in
./proj < tests/teste01.in > myoutput.out
diff myoutput.out tests/teste01.out
```

### Resultados possíveis de avaliação

| Resultado | Descrição |
|-----------|-----------|
| ✅ Accepted | O resultado do programa é igual ao esperado |
| ❌ Wrong Answer | O resultado do programa é diferente do esperado |
| ⚠️ Presentation Error | Difere do esperado em espaços ou linhas em branco |
| 🔴 Compile Time Error | Ocorreu um erro de compilação |
| ⏱️ Time Limit Exceeded | O tempo de execução excedeu o limite permitido |
| 💾 Memory Limit Exceeded | A memória excedeu o limite permitido |
| 📤 Output Limit Exceeded | O output excedeu o espaço permitido |

---

## 📋 Comandos Suportados

| Comando | Descrição |
|---------|-----------|
| `p <ean> <iva> <preço> <qty> <desc>` | Adicionar/atualizar produto |
| `l [padrão]` | Listar produtos |
| `a [qty] <ean>` | Adicionar/remover do cesto |
| `f [nif] [nome]` | Faturar e gerar fatura |
| `c [prefixo]` | Listar faturas por prefixo de nome |
| `r [ean]` | Relatório / estatísticas |
| `d <ean/id> [qty]` | Remover produto ou fatura |
| `u <nome-antigo> <nome-novo>` | Alterar nome de cliente nas faturas |
| `q` | Terminar programa |

---

## 🚀 Implementação DevOps

Para além do desenvolvimento do sistema em C, este projeto foi enriquecido com práticas modernas de **DevOps**, integrando ferramentas profissionais usadas na indústria.

### 🔁 CI/CD com GitHub Actions

Foi criado um pipeline automático (`.github/workflows/ci.yml`) que é acionado automaticamente a cada `git push` para o GitHub. O pipeline executa as seguintes etapas:

| Etapa | O que faz |
|-------|-----------|
| 🔨 **Build** | Compila com `gcc -O3 -Wall -Wextra -Werror` — falha se houver erros |
| 🧪 **Test** | Corre todos os testes públicos comparando output real vs esperado |
| 🔍 **Valgrind** | Verifica automaticamente erros e fugas de memória |
| 🐳 **Docker** | Constrói e valida a imagem Docker do projeto |

Isto garante que a cada alteração feita ao código, tudo é validado automaticamente — sem necessidade de testes manuais.

### 🐳 Containerização com Docker

Foi criado um `Dockerfile` com **multi-stage build**:

- **Fase 1 (Builder):** usa a imagem oficial `gcc:12` para compilar o projeto com as flags do IST
- **Fase 2 (Runtime):** copia apenas o binário final para uma imagem Ubuntu leve, sem ferramentas desnecessárias

```bash
# Build da imagem
docker build -t factuc-iaed .

# Correr em modo interativo
docker run --rm -it factuc-iaed

# Correr com ficheiro de input
docker run --rm -i factuc-iaed < tests/teste01.in
```

Isto permite correr o programa em **qualquer máquina** sem instalar compiladores ou dependências.

### 📊 Impacto da implementação DevOps

| Sem DevOps | Com DevOps |
|-----------|-----------|
| Testes feitos manualmente | Testes correm automaticamente a cada push |
| Erros de memória descobertos tarde | Valgrind corre automaticamente no pipeline |
| Projeto só corre no PC do autor | Corre em qualquer máquina via Docker |
| README simples | README profissional com badges dinâmicos |

---

## 👨‍💻 Autor

**Eduardo João Vianga**
- 🎓 BSc Computer Science & Engineering — Instituto Superior Técnico
- 💼 [LinkedIn](https://www.linkedin.com/in/eduardovianga23-j)
- 🐙 [GitHub](https://github.com/eduardovianga23-j)
