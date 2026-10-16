# Sincronização do Metrô

Este projeto implementa um sistema de sincronização de embarque para vagões de metrô automatizados usando threads POSIX. Cada vagão e cada passageiro são controlados por uma thread independente. O sistema garante embarque ordenado: o vagão não parte antes de todos os passageiros sentados, passageiros não embarcam sem vaga disponível, e múltiplos passageiros podem embarcar simultaneamente. A sincronização é feita exclusivamente com mutex e variáveis de condição POSIX, sem semáforos ou busy-wait.

## 💻 Sistema Operacional de Implementação e Suporte

Este projeto foi desenvolvido e testado no wsl (Windows Subsystem for Linux) usando **Ubuntu 24.04 LTS (Linux)**.

O código foi escrito em C padrão e é totalmente compatível e executável nativamente em ambientes **Linux, Unix e macOS**.

## 📂 Descrição dos Arquivos Utilizados

* **`sync_car.h`**: Arquivo de cabeçalho. Define a `struct station` com um único mutex (`pthread_mutex_t lock`), duas variáveis de condição (`car_arrived` e `passenger_boarded`) e os três campos de estado (`waiting_passengers`, `available_seats`, `boarding_passengers`). Declara também os protótipos das quatro funções públicas.
* **`sync_car.c`**: Contém a implementação das quatro funções do sistema:
  * `station_init` — inicializa todos os campos e as primitivas de sincronização.
  * `station_load_car` — chamada pelo vagão ao abrir as portas; usa `pthread_cond_broadcast` para acordar os passageiros e `pthread_cond_wait` para bloquear sem busy-wait até todos estarem sentados.
  * `station_wait_for_car` — chamada pelo passageiro ao chegar; dorme até haver vaga, reserva o assento e libera o lock antes de retornar, permitindo embarque simultâneo.
  * `station_on_board` — chamada pelo passageiro após sentar; sinaliza o vagão quando o último passageiro terminar de embarcar.
* **`sync_car-runner.c`**: Programa de testes com múltiplos cenários de validação. O professor pode substituir este arquivo por testes automatizados próprios, pois o Makefile compila este nome por padrão.
* **`Makefile`**: Script para automação da compilação. Gerencia as dependências entre os arquivos `.c` e `.h` e gera o executável `sync_car` na raiz do projeto.

## ⚙️ Como Compilar

Certifique-se de ter o compilador `gcc` e a ferramenta `make` instalados. Na raiz do projeto, utilize o seguinte comando:

**Para compilar o projeto:**
```
make
```

## Para limpar os arquivos objetos e o executável:
```
make clean
```

# ▶️ Como Executar

```
./sync_car
```

# 🧪 Como Testar o Programa

1 - Compile o projeto executando `make`.

2 - Execute o programa com `./sync_car`.

## Cenários testados:
- **Passageiros chegam antes do vagão:** threads dormem na condição `car_arrived` e são acordadas pelo `broadcast` do vagão.
- **Mais passageiros do que assentos:** o primeiro vagão parte cheio; os restantes aguardam o próximo.
- **Mais assentos do que passageiros:** o vagão parte com lugares vazios assim que todos embarcam.
- **Embarque simultâneo:** múltiplos passageiros retornam de `station_wait_for_car` antes de qualquer um chamar `station_on_board`.
- **Estação vazia:** vagão chega sem ninguém esperando e parte imediatamente.