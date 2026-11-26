# Tutorial de instalação do LUME (para iniciantes)

Este guia parte do zero. Siga as etapas na ordem.

---

## Etapa 1: Instalar os programas necessários (só uma vez)

O LUME é um plugin feito em **C++**. A Unreal precisa compilá-lo (transformar o código em programa),
e para isso você precisa de dois programas.

### 1.1 Unreal Engine 5.7
1. Abra o **Epic Games Launcher**.
2. Vá em **Unreal Engine → Biblioteca**.
3. Clique no **+** ao lado de "Versões do Motor", escolha **5.7** e clique em **Instalar**.

### 1.2 Visual Studio 2022 Community (gratuito)
1. Baixe em: https://visualstudio.microsoft.com/pt-br/vs/community/
2. Abra o instalador. Na aba **Cargas de trabalho**, marque:
   - ✅ **Desenvolvimento para desktop com C++**
   - ✅ **Desenvolvimento de jogos com C++**. No painel da direita, marque também **Instalador do Unreal Engine**.
3. Na aba **Componentes individuais**, confira se estão marcados:
   - ✅ **MSVC v143 – VS 2022 C++ x64/x86** (a versão mais recente)
   - ✅ **SDK do Windows 11** (ou do Windows 10), a versão mais recente
   - ✅ **.NET Framework 4.8 SDK** (ou o mais recente que aparecer)
4. Clique em **Instalar**. Leva um tempo. Ao terminar, **reinicie o computador**.

> Você não vai precisar programar nada. O Visual Studio só precisa estar instalado.

---

## Etapa 2: Ter um projeto C++

O plugin só compila em projetos que tenham C++.

### Se você vai criar um projeto novo
1. Na tela inicial da Unreal 5.7, escolha **Games → Third Person** (por exemplo).
2. Em **Project Defaults**, selecione **C++** (e não Blueprint).
3. Dê um nome, por exemplo `MeuJogo`, e clique em **Create**.
4. Espere abrir e depois **feche a Unreal**.

### Se o seu projeto já existe e é só Blueprint
1. Abra o projeto na Unreal.
2. Menu **Tools → New C++ Class...**
3. Escolha **None** (ou "Actor"), clique em **Next** e depois em **Create Class**.
4. A Unreal vai compilar e talvez abrir o Visual Studio. Pode fechar o Visual Studio.
5. Pronto, agora o projeto aceita C++. **Feche a Unreal.**

---

## Etapa 3: Baixar o plugin

1. Abra a página do repositório no GitHub: `github.com/codvision/LUME-Cinematic-Camera-System`
2. No botão de branch (canto superior esquerdo, onde aparece `main`), escolha **`claude/dazzling-ride-432oj9`**.
3. Clique no botão verde **Code → Download ZIP**.
4. Extraia o ZIP (botão direito → **Extrair tudo**).

---

## Etapa 4: Colocar o plugin dentro do projeto

1. Abra a pasta do seu projeto, onde está o arquivo **`MeuJogo.uproject`**.
   Normalmente fica em `Documentos\Unreal Projects\MeuJogo`.
2. Se não existir, crie uma pasta chamada **`Plugins`** (com P maiúsculo) ao lado do `.uproject`.
3. Copie a pasta extraída para dentro de `Plugins` e **renomeie-a** para `LumeCinematicCamera`.

O resultado deve ficar **exatamente** assim:

```
MeuJogo/
├── MeuJogo.uproject
├── Content/
├── Source/
└── Plugins/
    └── LumeCinematicCamera/
        ├── LumeCinematicCamera.uplugin   ← este arquivo tem que estar AQUI
        ├── README.md
        ├── Config/
        └── Source/
```

> ⚠️ Erro mais comum: ficar com uma pasta a mais, por exemplo
> `Plugins/LumeCinematicCamera/LUME-Cinematic-Camera-System-.../LumeCinematicCamera.uplugin`.
> O arquivo `.uplugin` precisa estar logo dentro de `Plugins/LumeCinematicCamera/`.

---

## Etapa 5: Compilar

### Jeito fácil
1. Dê **dois cliques** no `MeuJogo.uproject`.
2. Vai aparecer: *"The following modules are missing or built with a different engine version... Would you like to rebuild them now?"*
3. Clique em **Sim / Yes**.
4. Espere (pode levar alguns minutos). A Unreal abre sozinha quando terminar.

### Se aparecer "could not be compiled. Try rebuilding from source manually"
Faça a compilação pelo Visual Studio para ver o erro:
1. Clique com o botão direito no `MeuJogo.uproject` → **Generate Visual Studio project files**.
   No Windows 11, clique antes em **Mostrar mais opções**.
2. Vai aparecer o arquivo **`MeuJogo.sln`**. Abra-o com dois cliques (abre o Visual Studio).
3. No topo, confira se está selecionado **Development Editor** e **Win64**.
4. Menu **Compilação → Compilar Solução** (ou `Ctrl + Shift + B`).
5. Se der erro, abra **Exibir → Lista de Erros**, copie as mensagens e envie para mim, que eu corrijo.
6. Se aparecer **"Compilação: 1 com êxito"**, feche o Visual Studio e abra o `.uproject` normalmente.

---

## Etapa 6: Confirmar que o plugin está ativo

1. Na Unreal, vá em **Edit → Plugins**.
2. Pesquise **LUME**. Deve aparecer **LUME Cinematic Camera System** com a caixa marcada.
3. Se não estiver marcada, marque e reinicie a Unreal.

---

## Etapa 7: Usar a câmera pela primeira vez

1. Abra o painel **Place Actors**: menu **Window → Place Actors**, ou o botão **+** na barra de cima.
2. Pesquise **LUME**. Vão aparecer:
   - **LUME Cinematic Camera**
   - **LUME Camera Zone**
   - **LUME Camera Bounds**
3. Arraste uma **LUME Cinematic Camera** para qualquer lugar do mapa. A posição não importa.
4. Clique em **Play** (▶). A câmera passa a seguir o seu personagem.

**Ajustes básicos.** Selecione a câmera no mapa e, no painel **Details**, procure a seção **LUME | Framing**:
- **Distance**: distância até o personagem.
- **Field Of View**: zoom da lente.
- **Rotation**: ângulo. Em *Pitch*, valores negativos fazem a câmera olhar mais de cima.
- **Screen Offset**: posição do personagem na tela. X = 0.33 deixa ele à direita.

> 💡 Ajuste os valores **enquanto o jogo está rodando** para ver o efeito na hora.
> Mas o que você muda durante o Play é desfeito ao parar. Anote os valores e coloque-os de novo depois.

**Jogo com câmera controlada pelo mouse (terceira pessoa).** Na lista **LUME | Modules**, abra
**Rotation** e mude **Mode** para **Control Rotation**.

---

## Etapa 8: Criar zonas (corredor, sala grande, terror)

1. Arraste uma **LUME Camera Zone** para o mapa.
2. Posicione e redimensione a caixa laranja no ambiente. Selecione o componente **ZoneBox** e mude o **Box Extent**.
3. No Details, em **LUME | Framing → Preset**, escolha **Corredor**, **Sala Grande** ou **Terror**.
4. Dê **Play** e ande para dentro da caixa. A câmera muda suavemente e, ao sair, volta ao normal.

Extras da zona:
- **Use Fixed Viewpoint**: a câmera vai para um ponto fixo da sala (a seta vermelha) e acompanha o
  jogador girando. Mova o componente **FixedViewpoint** para posicionar esse ponto.
- **Constrain Camera To Zone**: a câmera não sai da caixa.
- **Camera To Activate**: troca para outra LUME Camera ao entrar. Nessa outra câmera, **desmarque** `Auto Activate`.

## Etapa 9: Limitar a câmera (opcional)

Arraste uma **LUME Camera Bounds** e ajuste a caixa azul. A câmera nunca sai dela.

---

## Problemas comuns

| Problema | Solução |
|---|---|
| "Missing modules" e não compila | Visual Studio não instalado ou faltam as cargas de trabalho C++ (Etapa 1). Ou o projeto não tem C++ (Etapa 2). |
| O plugin não aparece em Edit → Plugins | O `.uplugin` está na pasta errada (Etapa 4). |
| Não acho "LUME" no Place Actors | O plugin não compilou. Refaça a Etapa 5. |
| A câmera não assume a visão | Verifique se `Auto Activate` está marcado e se só **uma** câmera LUME tem essa opção marcada. |
| A câmera segue o personagem errado | Em **LUME / Target**, escolha o **Target Actor** manualmente. |
| A zona não faz efeito | O personagem (o centro da cápsula) precisa estar **dentro** da caixa. Confira se `Enabled` está marcado. |
| Quero ver o que a câmera está fazendo | Marque **Draw Debug** na câmera. Aparecem esferas, setas e o peso das zonas no canto da tela. |

Se travar em qualquer etapa, mande o print ou o texto do erro.
