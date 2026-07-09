# Jogo desenvolvido na disciplina de Programação de Sistemas do curso de Ciência da Computação.
Nome: Giovana da Silva Bulcão/ra144325 e Cintia da Silva Bulcão/ra145107

Especificação do Protótipo:
Nome do jogo: Maze escape
O objetivo do jogador é explorar um labirinto, coletar os 5 fragmentos espalhados pelo mapa e, em seguida, encontrar a saída, desviando dos inimigos.

Controles: as setas movimentam o jogador, e o X inicia o jogo e tambem coleta os fragmentos.

O jogo possui quatro telas principais: Menu Inicial, Dungeon, Game Over e Tela de Vitória. durante a partida, o jogador se move pelo labirinto evitando paredes, os fragmentos desaparecem após serem coletados e caso um inimigo alcance o jogador, o jogo termina.
Se todos os fragmentos forem coletados e o jogador alcançar a saída, a vitória é exibida.

Introdução e especificações da plataforma WASM-4:

O WASM-4 é um console de jogos de fantasia de baixo nível para a construção de pequenos jogos com WebAssembly. Os consoles de fantasia forçam os desenvolvedores a trabalhar dentro das limitações de tecnologia e design. A ideia é que por removendo o excesso e focando no essencial, torna-se mais fácil começar e terminar de desenvolver um jogo.
Especificações de Hardware:

Display: 160x160 pixels, 4 cores personalizáveis, atualizado a 60 Hz.
Memória: 64 KB de RAM linear, E/S mapeada por memória, salvar estados.
Limite de tamanho do cartucho: 64 KB.
Entrada: Teclado, mouse, tela sensível ao toque, até 4 gamepads.
Áudio: 2 canais de onda de pulso, 1 canal de onda triângulo, 1 canal de ruído.
Armazenamento de disco: 1024 bytes.

