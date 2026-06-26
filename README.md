# Tamaguchi Dungeon

A game written in C for the [WASM-4](https://wasm4.org) fantasy console.

## Building

Build the cart by running:

```shell
make
```

Then run it with:

```shell
w4 run build/cart.wasm
```

For more info about setting up WASM-4, see the [quickstart guide](https://wasm4.org/docs/getting-started/setup?code-lang=c#quickstart).

## Links

- [Documentation](https://wasm4.org/docs): Learn more about WASM-4.
- [Snake Tutorial](https://wasm4.org/docs/tutorials/snake/goal): Learn how to build a complete game
  with a step-by-step tutorial.
- [GitHub](https://github.com/aduros/wasm4): Submit an issue or PR. Contributions are welcome!

# Jogo desenvolvido na disciplina de Programação de Sistemas do curso de Ciência da Computação.

Introdução e especificações da plataforma WASM-4:

O WASM-4 é um console de jogos de fantasia de baixo nível para a construção de pequenos jogos com WebAssembly. Os consoles de fantasia forçam os desenvolvedores a trabalhar dentro das limitações de tecnologia e design. A ideia é que por removendo o excesso e focando no essencial, torna-se mais fácil começar e terminar de desenvolver um jogo.
Especificações de Hardware:

Display: 160x160 pixels, 4 cores personalizáveis, atualizado a 60 Hz.
Memória: 64 KB de RAM linear, E/S mapeada por memória, salvar estados.
Limite de tamanho do cartucho: 64 KB.
Entrada: Teclado, mouse, tela sensível ao toque, até 4 gamepads.
Áudio: 2 canais de onda de pulso, 1 canal de onda triângulo, 1 canal de ruído.
Armazenamento de disco: 1024 bytes.
Joguinho desenvolvido por mim e uma colega. História: O mundo acabou. Ninguém sabe bem como, um dia as luzes simplesmente apagaram, e o que sobrou foram ruínas, sombras e silêncio. Você vaga por esse mundo sozinho há tempo demais. Até que, num templo esquecido, você encontra um ovo pulsando no escuro. Ele racha. E sai uma criaturinha estranha e fofa. Ela te olha sem medo nenhum, como se já te conhecesse. Você não sabe o que ela é. Mas decide cuidar dela. De dia você entra nas dungeons que tomaram conta do mundo, buscando comida e o que mais puder carregar. De noite você volta pro abrigo, alimenta o bichinho, e ela dorme pertinho de você. Aos poucos você percebe: quando a criaturinha está feliz, o escuro parece um pouco menos escuro. Talvez seja só impressão. Talvez não. Por enquanto, basta isso.

Protótipo:

Tela inicial: Capa e inicio do jogo

Tela 1: Dungeon, mapa fixo, jogador se move pelos 4 direcionais, 1 tipo de inimigo que patrulha(vai e volta), colisões com paredes, itens de comida espalhados para coleta, ao coletar tudo, volta pra tela 2.

Tela 2: Abrigo do bichinho. bichinho aparece no centro, medidores de fome e felicidade, botão X alimenta(gasta comida coletada, desce fome), botão Z faz carinho, expressão do bichinho muda e botão para voltar a dungeon.

Fim de jogo: Se a fome ou felicidade chegar a zero, o bichinho some e a escuridão volta.

Vitória: ao coletar os fragmentos de Eco espalhados nas dungeons e manter o bichinho feliz, ele brilha e o escuro recua.
