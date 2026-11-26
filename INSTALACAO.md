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

## Etapa 7: Colocar a câmera no personagem

1. No **Content Browser**, abra a Blueprint do seu personagem.
   No template Third Person ela fica em `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`.
2. No painel **Components** (canto superior esquerdo), clique em **+ Add**.
3. Pesquise **LUME** e escolha **LUME Cinematic Camera**.
   Tanto faz onde ela fica na lista, porque a câmera não herda o movimento do personagem.
4. Clique em **Compile** e depois em **Save** (botões no topo da Blueprint).
5. Clique em **Play ▶**. A câmera LUME passa a seguir o personagem.
   A câmera antiga do template (`FollowCamera`) é desligada sozinha. Você pode deixá-la ou apagá-la.

**Onde ficam as configurações.** Na Blueprint do personagem, clique no componente **LumeCinematicCamera**.
O painel **Details** (à direita) mostra tudo:
- **LUME | Framing**
  - **Distance**: distância até o personagem.
  - **Field Of View**: zoom da lente.
  - **Rotation**: ângulo. Em *Pitch*, valores negativos fazem a câmera olhar mais de cima.
  - **Screen Offset**: posição do personagem na tela. X = 0.33 deixa ele à direita.
- **LUME | Modules**: os módulos (rastreamento, zoom, profundidade de campo...). Clique na setinha de cada um para abrir as opções.
- **LUME | Target / Activation / Debug**: alvo, ativação e depuração.

**Controles.** No modo padrão (**World Fixed**, câmera com ângulo fixo), a opção
**Align Controls To Camera** faz "W / analógico para frente" sempre andar "para dentro da tela".
Não precisa mexer em nada na Blueprint do personagem.

**Câmera controlada pelo mouse (terceira pessoa clássica).** Em **LUME | Modules → Rotation**, mude
**Mode** para **Control Rotation**.

> 💡 Para testar valores rápido: dê Play, aperte **F8** (solta o mouse do jogo), selecione o
> personagem no **Outliner**, clique no componente LUME e mude os valores. O efeito aparece na hora.
> O que você muda durante o Play é desfeito ao parar, então anote os valores e coloque-os depois na Blueprint.

### Alternativa: câmera solta no mundo
Arraste o ator **LUME Cinematic Camera** (painel **Place Actors**) para o mapa. Isso serve para
câmeras secundárias, ativadas por zonas ou cutscenes. Se quiser que esse ator seja a câmera
principal, marque **Take Over Player View** nele, e então não adicione o componente no personagem.

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
- **Camera To Activate**: troca para um ator LUME Cinematic Camera do mapa ao entrar e volta para a câmera do personagem ao sair.

## Etapa 9: Limitar a câmera (opcional)

Arraste uma **LUME Camera Bounds** e ajuste a caixa azul. A câmera nunca sai dela.

---

## Etapa 10: Configurar para jogo 2.5D (lateral)

Clique no componente **LumeCinematicCamera** na Blueprint do personagem e ajuste:

**Direção da câmera** (**LUME | Framing → Rotation → Yaw**):
- Se o personagem anda para os lados no eixo **Y** (seta verde no editor): **Yaw = 0**.
- Se anda no eixo **X** (seta vermelha): **Yaw = 90** (ou -90 para ver do outro lado).

**Valores recomendados para 2.5D:**

| Onde | Campo | Valor |
|---|---|---|
| LUME \| Framing | Distance | 900 |
| LUME \| Framing | Field Of View | 50 (menos distorção nas bordas) |
| LUME \| Framing | Pivot Offset | Z = 80 |
| LUME \| Framing | Screen Offset | X = 0, Y = -0.1 (personagem um pouco abaixo do centro) |
| LUME \| Framing | Rotation | Pitch = -8 |
| LUME \| Framing | Look Ahead Distance | 250 (mostra mais à frente de onde ele corre) |
| LUME \| Framing | Dead Zone | X = 40, Y = 90 (pulos pequenos não mexem a câmera) |
| LUME \| Framing | Aperture | 8 |
| Modules → Rotation | Mode | World Fixed |
| Modules → Collision | Enabled | **desmarcado** (em 2.5D, objetos na frente puxariam a câmera) |

**Para o personagem não sair da "linha" do jogo:** selecione o componente **Character Movement**
(Componente de Movimento do Personagem). Em **Planar Movement**, marque **Constrain to Plane** e,
em **Plane Constraint Axis Setting**, escolha o eixo de profundidade (X, se ele anda em Y).

**Limites da fase:** coloque um **LUME Camera Bounds** cobrindo a área onde a câmera pode ir. Ela
para nas bordas, como nos jogos de plataforma clássicos.

---

## Etapa 11: Tremor de câmera (golpes, explosões, quedas)

**Aterrissagem:** é automática. Quedas altas já dão um tranco na câmera.

**Em qualquer Blueprint** (inimigo, armadilha, Level Blueprint), clique com o botão direito no grafo e procure:
- **LUME Shake Camera**: escolha o **Preset** (Golpe Leve, Golpe Forte, Explosão, Aterrissagem, Susto) e a **Scale** (força, 1 = normal).
  Exemplo: no evento de dano do personagem → **LUME Shake Camera** com **Golpe Forte**.
- **LUME Shake At Location**: tremor com origem num ponto (explosão). Ligue a posição da explosão em **Location**.
  Perto treme forte, longe treme menos, e fora do **Outer Radius** não treme.
- **LUME Set Continuous Shake**: tremor contínuo (terremoto, máquina). **Amount** 0.3 a 0.5 liga, e 0 desliga.
- **LUME Camera Impulse**: empurrão numa direção. A câmera volta quicando, como uma mola.

**Ajustes** em **LUME | Modules → Shake & Impacts (Tremor)**:
- **Global Intensity**: força de todos os tremores. Bom para colocar no menu de opções (0 = desligado).
- **Max Offset / Max Angles**: o quanto a câmera se mexe no tremor mais forte.
- **Landing Speed Threshold**: a velocidade de queda a partir da qual há tranco.

---

## Etapa 12: Pontos de interesse e olhar em volta

### Pontos de interesse
Servem para fazer a câmera "dar atenção" a uma porta, um baú, um inimigo ou um chefão quando o jogador chega perto.

**No mapa:** arraste um **LUME Camera Interest Point** (painel Place Actors). A esfera **verde** é
onde começa a atração, e a **amarela** é onde a atração é total.

**Num objeto com Blueprint** (inimigo, porta): abra a Blueprint, clique em **+ Add** e escolha **LUME Interest Point**.

Opções principais:
- **Influence**: quanto a câmera vai na direção do ponto (0.3 é sutil, 0.7 é forte).
- **Keep In Frame**: afasta a câmera para mostrar o jogador e o ponto juntos. Ótimo para chefões.
- **Extra Distance**: afasta a câmera enquanto o jogador está perto (um valor negativo aproxima).

### Olhar em volta (espiar)
Deixa o jogador segurar um botão ou o analógico direito para espiar para os lados, para cima ou para baixo.

1. Na Blueprint do personagem, crie (ou use) uma **Input Action** do tipo **Axis2D (Vector2D)** ligada ao
   analógico direito e/ou às setas.
2. No grafo, no evento **Triggered** dessa ação, ligue o valor (**Action Value**) no nó **LUME Set Look Input**.
3. Pronto. Enquanto o botão estiver pressionado, a câmera espia, e quando o jogador solta, ela volta sozinha.

Para espiar só para cima e para baixo (clássico de plataforma), ligue só o Y. Para isso, use
**Make Vector 2D** com X = 0.

A distância da espiada fica em **LUME | Modules → Interest & Look Around → Look Around Distance**.

---

## Problemas comuns

| Problema | Solução |
|---|---|
| "Missing modules" e não compila | Visual Studio não instalado ou faltam as cargas de trabalho C++ (Etapa 1). Ou o projeto não tem C++ (Etapa 2). |
| O plugin não aparece em Edit → Plugins | O `.uplugin` está na pasta errada (Etapa 4). |
| Não acho "LUME" no + Add nem no Place Actors | O plugin não compilou. Refaça a Etapa 5. |
| Continua aparecendo a câmera antiga | No componente LUME, confira se **Take Over Player View** e **Deactivate Other Cameras** estão marcados. Se houver um ator LUME no mapa com **Take Over Player View** marcado, desmarque. |
| Andar para frente vai para o lado errado | No componente LUME, marque **Align Controls To Camera** (funciona no modo World Fixed). |
| A câmera segue o personagem errado | Em **LUME / Target**, escolha o **Target Actor** manualmente. |
| A zona não faz efeito | O personagem (o centro da cápsula) precisa estar **dentro** da caixa. Confira se `Enabled` está marcado. |
| Não vejo "Shake & Impacts" ou "Interest & Look Around" na lista de módulos | É normal em câmeras criadas antes desta versão. Eles são adicionados sozinhos ao dar Play. Para ajustá-los no editor, clique no **+** da lista **Modules** e adicione o módulo. |
| A câmera pula para perto do personagem em 2.5D | Desmarque o módulo **Collision** (Etapa 10). |
| Quero ver o que a câmera está fazendo | Marque **Draw Debug** no componente LUME. Aparecem esferas, setas e o peso das zonas no canto da tela. |

Se travar em qualquer etapa, mande o print ou o texto do erro.
