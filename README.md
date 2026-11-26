# LUME Cinematic Camera System

Sistema **modular** de câmera cinematográfica para **Unreal Engine 5.7** (C++ e Blueprint).

A câmera é um **ator independente**: não fica presa ao personagem nem a um spring arm no mesh.
Ela só observa um alvo e roda uma lista de módulos (um "pipeline"). Você pode ligar, desligar,
reordenar e trocar os módulos no editor, ou criar novos em C++ ou Blueprint.

---

## Instalação

1. Copie esta pasta para `SeuProjeto/Plugins/LumeCinematicCamera/`.
2. Gere os arquivos do projeto (botão direito no `.uproject` → *Generate Visual Studio project files*) e compile.
3. No editor: **Edit → Plugins → LUME Cinematic Camera System** (deve aparecer ativado).

> Para projetos só de Blueprint: adicione uma classe C++ qualquer ao projeto uma vez, para que o
> plugin seja compilado.

## Uso rápido

1. Arraste um **LUME Cinematic Camera** para o nível.
2. Deixe ligados `Auto Target Player Pawn` e `Auto Activate`. A câmera vai seguir o jogador e assumir a visão.
3. Ajuste o **Base Framing**: distância, FOV, rotação, posição na tela etc.
4. Coloque **LUME Camera Zones** nos ambientes e escolha um preset (Corredor, Sala Grande, Terror).
5. Opcional: coloque **LUME Camera Bounds** para limitar por onde a câmera pode andar.
6. Ligue `Draw Debug` na câmera para ver pivô, zona morta, antecipação e o peso de cada zona.

---

## Arquitetura

```
ALumeCinematicCamera  (ator independente, tick em PostPhysics, depois do movimento do personagem)
 ├─ Alvo: TargetActor / pawn do jogador, com referência configurável
 ├─ BaseFraming  ─┐
 ├─ Zonas ────────┼─► mistura por prioridade com SmoothStep ─► Context.Framing
 ├─ Overrides ────┘      (scripts: PushFramingOverride)
 └─ Modules[] (Instanced, editáveis no Details)
      PreCompose : Tracking → Framing → Rotation → Dynamic Zoom
      Composição  : pivô + deslocamento de tela + rotação + distância (+ ponto de vista fixo)
      PostCompose: Collision → Bounds → Depth of Field → Handheld Noise
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
| Câmera independente das animações | Ator separado, rastreia a **localização do ator/cápsula** (não o mesh), compensa o agachamento (`Compensate Crouch`), estabiliza pulos e mede a velocidade pela posição. |
| Referência do jogador configurável | `Target Actor` (escolha no editor), `Auto Target Player Pawn` + `Player Index`, `Tracking Reference` (Actor Location, Named Component ou Socket/Bone), `PivotOffset`. |
| Colisão | `Collision`: sphere sweep. Aproxima na hora ao bater em algo e volta suavemente. |
| Câmera na mão | `Handheld Noise`: ruído Perlin. Amplitude e frequência vêm do enquadramento. |

---

## API de Blueprint (principais nós)

**LUME Cinematic Camera**
- `Set Target(Actor, bSnap)`, `Get Target`, `Snap To Target`
- `Activate Camera(BlendTime, BlendFunction, BlendExponent)`
- `Push Framing Override(Overrides, Priority, BlendIn, BlendOut)` → `Handle`; `Pop Framing Override(Handle)`; `Clear Framing Overrides`
  - Útil em cutscenes e sustos: por exemplo, aproximar e fechar o FOV por 2 segundos.
- `Get Current Framing`, `Get Camera State`, `Get Zone Weight(Zone)`
- `Find Module By Class`, `Add Module`

**LUME Camera Subsystem** (nó *Get LumeCameraSubsystem*)
- `Activate Camera`, `Push Camera`, `Pop Camera`, `Get Active Camera`
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
- **2.5D / plataforma lateral**: `World Fixed` com `Rotation = (0, 90, 0)`, `DeadZone = (60, 80)`, `LookAheadDistance = 250` e LUME Camera Bounds nos limites da fase.
- **Survival horror de câmeras fixas**: uma zona por sala com `Use Fixed Viewpoint` e o preset Terror. Posicione o `FixedViewpoint` no canto da sala.
- **Câmera ignora o personagem em cutscene**: `Set Target` para um ator da cena com `bSnap = false`, para a câmera viajar até ele.

## Estrutura

```
LumeCinematicCamera.uplugin
Source/LumeCinematicCamera/
  Public/
    LumeCinematicCameraActor.h   ALumeCinematicCamera
    LumeCameraZone.h             ALumeCameraZone
    LumeCameraZoneProfile.h      ULumeCameraZoneProfile (Data Asset)
    LumeCameraBoundsVolume.h     ALumeCameraBoundsVolume
    LumeCameraSubsystem.h        ULumeCameraSubsystem
    LumeCameraTypes.h            Enquadramento, overrides, presets, estado, contexto
    LumeCameraMath.h             SmoothDamp e utilidades de caixa
    Modules/                     Tracking, Framing, Rotation, DynamicZoom, Collision,
                                 Bounds, DepthOfField, HandheldNoise, Blueprint Module
```
