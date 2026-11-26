# LUME Cinematic Camera System

Sistema **modular** de câmera cinematográfica para **Unreal Engine 5.7** (C++ e Blueprint).

A câmera é um **componente** (`LUME Cinematic Camera`) que você adiciona na Blueprint do personagem.
Todas as configurações aparecem no painel Details. Mesmo dentro do personagem, ela é **independente**:
usa transform absoluto, então cápsula, mesh e animações não a movem. Ela só observa um alvo e roda
uma lista de módulos (um "pipeline"). Você pode ligar, desligar, reordenar e trocar os módulos no
editor, ou criar novos em C++ ou Blueprint.

Também existe o **ator** `LUME Cinematic Camera`, que carrega o mesmo componente, para câmeras
soltas no mundo: câmeras secundárias, cutscenes, câmeras fixas.

---

## Instalação

> Iniciante? Siga o passo a passo completo em [INSTALACAO.md](INSTALACAO.md).

1. Copie esta pasta para `SeuProjeto/Plugins/LumeCinematicCamera/`.
2. Gere os arquivos do projeto (botão direito no `.uproject` → *Generate Visual Studio project files*) e compile.
3. No editor: **Edit → Plugins → LUME Cinematic Camera System** (deve aparecer ativado).

> Para projetos só de Blueprint: adicione uma classe C++ qualquer ao projeto uma vez, para que o
> plugin seja compilado.

## Uso rápido

1. Abra a Blueprint do personagem → **Add** → **LUME Cinematic Camera** (pode ficar em qualquer lugar da hierarquia).
2. Pronto: com `Auto Target` e `Take Over Player View` ligados, ela segue o personagem e assume a visão.
   As outras câmeras do personagem (ex.: a `FollowCamera` do template) são desativadas sozinhas (`Deactivate Other Cameras`).
3. Ajuste o **Base Framing** no Details do componente: distância, FOV, rotação, posição na tela etc.
4. Coloque **LUME Camera Zones** nos ambientes e escolha um preset (Corredor, Sala Grande, Terror).
5. Opcional: coloque **LUME Camera Bounds** para limitar por onde a câmera pode andar.
6. Ligue `Draw Debug` na câmera para ver pivô, zona morta, antecipação e o peso de cada zona.

---

## Arquitetura

```
ULumeCinematicCameraComponent  (UCameraComponent com transform absoluto, tick em PostPhysics;
                                vai no personagem ou dentro do ator ALumeCinematicCamera)
 ├─ Alvo: TargetActor / pawn do jogador, com referência configurável
 ├─ BaseFraming  ─┐
 ├─ Zonas ────────┼─► mistura por prioridade com SmoothStep ─► Context.Framing
 ├─ Overrides ────┘      (scripts: PushFramingOverride)
 └─ Modules[] (Instanced, editáveis no Details)
      PreCompose : Tracking → Framing → Interest → Rotation → Dynamic Zoom
      Composição  : pivô + deslocamento de tela + rotação + distância (+ ponto de vista fixo)
      PostCompose: Collision → Bounds → Depth of Field → Handheld Noise → Shake
ULumeCameraSubsystem  (registro de zonas, limites e câmeras; câmera ativa com pilha push/pop)
```

Toda a suavização usa uma **mola criticamente amortecida** (`LumeMath::SmoothDamp`). Ela não
depende do frame rate e não ultrapassa o destino, por isso a câmera não dá trancos.

Cada módulo pega os valores "crus" do enquadramento e os troca pela sua versão suavizada.
Se você remover um módulo, a câmera continua funcionando, só sem aquele comportamento.

---

## 1. Rastreamento inteligente: `Tracking`

| Recurso | Como funciona |
|---|---|
| Segue suavemente | Mola amortecida com `Framing.TrackingSmoothTime`. O eixo vertical é mais suave (`VerticalSmoothMultiplier`). |
| Antecipa o movimento | A velocidade é medida pela posição do alvo e suavizada. O pivô avança até `Framing.LookAheadDistance` na direção do movimento. |
| Posição cinematográfica | Zona morta (`Framing.DeadZone`) no plano da tela, combinada com `Framing.ScreenOffset` (módulo Framing). |
| Evita movimentos bruscos | `StableWhileAirborne` ignora pulos e só segue quedas maiores que `AirborneVerticalTolerance`. Detecta teleportes. |

## 2. Câmera dinâmica

- **Zoom automático** (`Dynamic Zoom`): afasta a câmera e abre o FOV conforme a velocidade. Aproxima devagar quando o jogador fica parado (*idle push-in*).
- **Ajuste de enquadramento** (`Framing`): transição suave da posição na tela e *enquadramento de grupo*. Adicione inimigos ou objetos em `AdditionalTargets` (ou use `AddFramingTarget` em Blueprint) e a câmera desloca o pivô e se afasta para todos caberem na tela.
- **Deslocamento horizontal/vertical**: `Framing.ScreenOffset`, de -1 a 1. Por exemplo, `(0.33, 0)` é a regra dos terços. `PivotOffset` define o ponto mirado (Z=50 mira no peito).
- **Rotação suave** (`Rotation`): modos `World Fixed`, `Follow Target Heading`, `Follow Movement Direction` e `Control Rotation`. Mudanças de ângulo são sempre interpoladas.
  Em `World Fixed`, `Align Controls To Camera` gira a Control Rotation junto com a câmera, então "frente" no controle é sempre "para dentro da tela" sem mudar a Blueprint do personagem.

## 3. Zonas cinematográficas: `LUME Camera Zone`

- Cada zona tem **sobrescritas parciais**: só os campos marcados mudam. Também tem `Priority`, `Blend In Time` e `Blend Out Time`.
- **Presets**: escolha em `Preset` e o campo `Framing` é preenchido (dá para ajustar depois):
  - **Corredor**: câmera perto (340), FOV 60, pouca antecipação, zona morta pequena.
  - **Sala Grande**: câmera longe e alta (1150, pitch -28), FOV 78, muita antecipação, zoom lento.
  - **Terror**: personagem fora do centro (0.32, -0.12), dutch angle de 5°, FOV 52, foco raso (f/1.8), vinheta forte, granulação, câmera pesada e efeito de câmera na mão.
- **Perfis reutilizáveis**: crie um *Data Asset* do tipo **LUME Camera Zone Profile** (ex.: `DA_Terror`) e use o mesmo perfil em várias zonas.
- **Enquadramentos especiais**:
  - `Use Fixed Viewpoint`: a câmera vai até o componente `FixedViewpoint` e gira para acompanhar o jogador (estilo câmera de segurança ou survival horror clássico).
  - `Constrain Camera To Zone`: a câmera não sai do volume da zona.
  - `Camera To Activate`: troca para outra LUME Camera com transição. Ao sair da zona, volta para a anterior.
- Eventos `OnPlayerEnterZone` / `OnPlayerExitZone` para Blueprint.

O que decide se a zona está ativa é a **posição do alvo** dentro da caixa (a caixa pode estar
rotacionada). Por isso funciona com qualquer ator, mesmo sem colisão. Zonas sobrepostas se
somam por prioridade.

## 4. Recursos adicionais

| Recurso | Onde |
|---|---|
| Profundidade de campo | `Depth of Field`: foco automático no jogador com "puxar foco" suave. A abertura vem do enquadramento, então cada zona pode ter a sua. Também aplica vinheta e granulação. |
| Transições suaves entre câmeras | `ULumeCameraSubsystem::ActivateCamera / PushCamera / PopCamera` (blend com curva e expoente), `ALumeCinematicCamera::ActivateCamera`, ou `Camera To Activate` nas zonas. |
| Limites de movimentação | `LUME Camera Bounds`: vários volumes funcionam como uma união. O módulo `Bounds` tem margem e a opção de reapontar a câmera. Também respeita as zonas com restrição. |
| Câmera independente das animações | Transform absoluto (nem dentro do personagem herda movimento), rastreia a **localização do ator/cápsula** (não o mesh), compensa o agachamento (`Compensate Crouch`), estabiliza pulos e mede a velocidade pela posição. |
| Referência do jogador configurável | `Target Actor` (escolha no editor), `Auto Target` (o próprio personagem, ou o pawn do jogador + `Player Index` no ator do mundo), `Tracking Reference` (Actor Location, Named Component ou Socket/Bone), `PivotOffset`. |
| Colisão | `Collision`: sphere sweep. Aproxima na hora ao bater em algo e volta suavemente. |
| Câmera na mão | `Handheld Noise`: ruído Perlin. Amplitude e frequência vêm do enquadramento. |

---

## 5. Tremor e impactos: `Shake & Impacts`

- **Trauma**: cada evento soma trauma (0..1), que decai sozinho. O tremor é proporcional a trauma²,
  com ruído Perlin em posição (no plano da tela, ideal para 2.5D) e em rotação.
- **Empurrão com mola**: o impulso desloca a câmera e uma mola subamortecida a traz de volta.
- **Aterrissagem automática**: quedas acima de `Landing Speed Threshold` geram trauma e um tranco para baixo.
- **Presets**: Golpe Leve, Golpe Forte, Explosão, Aterrissagem, Susto. Também há tremor contínuo (terremoto).
- **Global Intensity**: opção de acessibilidade.

## 6. Pontos de interesse e olhar em volta: `Interest & Look Around`

- **LUME Interest Point** (componente para qualquer Blueprint) e **LUME Camera Interest Point** (ator com os raios visíveis no editor).
  Perto do ponto, o pivô desliza na direção dele (`Influence`, `MaxShift`), com queda suave entre `FullInfluenceRadius` e `ActivationRadius`.
  `Keep In Frame` afasta a câmera para mostrar o jogador e o ponto, e `Extra Distance` afasta ou aproxima.
- **Olhar em volta**: `Set Look Input(-1..1)` desloca a câmera no plano da tela até `LookAroundDistance`.
  Quando o jogador para de enviar entrada, ela volta sozinha.

## Migração de módulos

Com `Auto Add Missing Modules` (ligado por padrão), módulos padrão que faltem na lista são adicionados na
posição correta ao dar Play. Câmeras criadas antes de uma atualização ganham os módulos novos sem você mexer
em nada. Para desligar um módulo, desmarque `Enabled` nele em vez de removê-lo.

## API de Blueprint (principais nós)

**LUME Cinematic Camera** (componente; no ator use `Get Lume Camera`)
- `Set Target(Actor, bSnap)`, `Get Target`, `Snap To Target`
- `Activate Camera(BlendTime, BlendFunction, BlendExponent)`
- `Push Framing Override(Overrides, Priority, BlendIn, BlendOut)` → `Handle`; `Pop Framing Override(Handle)`; `Clear Framing Overrides`
  - Útil em cutscenes e sustos: por exemplo, aproximar e fechar o FOV por 2 segundos.
- `Get Current Framing`, `Get Camera State`, `Get Zone Weight(Zone)`, `Get Camera Yaw Rotation`
- `Find Module By Class`, `Add Module`

- `Play Shake(Preset, Scale, Direction)`, `Add Trauma`, `Add Camera Impulse`, `Set Continuous Shake`, `Set Look Input`

**Nós globais** (funcionam em qualquer Blueprint)
- `Get Active LUME Camera`, `LUME Shake Camera`, `LUME Shake At Location`, `LUME Camera Impulse`,
  `LUME Set Continuous Shake`, `LUME Set Look Input`

**LUME Camera Subsystem** (nó *Get LumeCameraSubsystem*)
- `Activate Camera`, `Push Camera`, `Pop Camera` (recebem qualquer ator com câmera), `Get Active Camera`, `Get Active Camera Actor`
- `Get All Cameras`, `Get All Zones`, evento `On Active Camera Changed`

## Criando seus próprios módulos

**C++**
```cpp
UCLASS(meta = (DisplayName = "Meu Módulo"))
class UMyLumeModule : public ULumeCameraModule
{
    GENERATED_BODY()
public:
    virtual ELumeModuleStage GetStage() const override { return ELumeModuleStage::PostCompose; }
    virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override
    {
        State.CameraLocation.Z += 10.0; // exemplo
    }
};
```

**Blueprint**: crie um Blueprint filho de **Blueprint Module** (`ULumeBlueprintCameraModule`),
escolha o `Stage`, implemente **Update Module** e retorne o `State` modificado. Depois adicione o
módulo na lista `Modules` da câmera.

---

## Receitas

- **Terceira pessoa com órbita livre**: em `Rotation`, use `Mode = Control Rotation`; `ScreenOffset = (0.25, 0)` deixa o personagem por cima do ombro.
- **2.5D / plataforma lateral**: `World Fixed` com Yaw perpendicular ao eixo do movimento, `Distance = 900`,
  `FOV = 50`, `Pitch = -8`, `DeadZone = (40, 90)`, `LookAheadDistance = 250` e o módulo **Collision desligado**.
  Use LUME Camera Bounds nos limites da fase, Interest Points em portas e chefões, `Set Look Input` no
  analógico direito e o tremor automático de aterrissagem. O passo a passo está na Etapa 10 de [INSTALACAO.md](INSTALACAO.md).
- **Survival horror de câmeras fixas**: uma zona por sala com `Use Fixed Viewpoint` e o preset Terror. Posicione o `FixedViewpoint` no canto da sala.
- **Câmera ignora o personagem em cutscene**: `Set Target` para um ator da cena com `bSnap = false`, para a câmera viajar até ele.

## Estrutura

```
LumeCinematicCamera.uplugin
Source/LumeCinematicCamera/
  Public/
    LumeCinematicCameraComponent.h ULumeCinematicCameraComponent (a câmera)
    LumeCinematicCameraActor.h   ALumeCinematicCamera (ator do mundo com o componente)
    LumeCameraZone.h             ALumeCameraZone
    LumeCameraZoneProfile.h      ULumeCameraZoneProfile (Data Asset)
    LumeCameraBoundsVolume.h     ALumeCameraBoundsVolume
    LumeCameraSubsystem.h        ULumeCameraSubsystem
    LumeInterestPointComponent.h ULumeInterestPointComponent, ALumeCameraInterestPoint
    LumeCameraLibrary.h          Nós globais de Blueprint (tremor, olhar em volta)
    LumeCameraTypes.h            Enquadramento, overrides, presets, estado, contexto
    LumeCameraMath.h             SmoothDamp e utilidades de caixa
    Modules/                     Tracking, Framing, Rotation, DynamicZoom, Collision,
                                 Bounds, DepthOfField, HandheldNoise, Interest, Shake, Blueprint Module
```
