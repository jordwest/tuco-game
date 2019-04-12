module Canvas = {
  type t;

  let find = (): t => [%bs.raw {|document.getElementById("canvas")|}];
  [@bs.send] external getContext: (t, string) => GL_extern.ctx = "getContext";
  let getWebGLContext = (t: t): GL_extern.ctx => getContext(t, "webgl");

  [@bs.send] external requestPointerLock: (t) => unit = "requestPointerLock";
};

module Document {
  type t;
  type evt;

  [@bs.send] external addEventListener: (t, string, (evt) => unit) => unit = "addEventListener";
  [@bs.get] external getMovementX: evt => float = "movementX";
  [@bs.get] external getMovementY: evt => float = "movementY";
  [@bs.get] external getKey: evt => string = "key";
}
[@bs.val] external document: Document.t = "document";

type window;
[@bs.val] external window: window = "window";
[@bs.send]
external requestAnimationFrame: (window, float => unit) => unit =
  "requestAnimationFrame";

module Game = {
  type point = (float, float, float);
  type element = {
    rotation: ref(float),
    scale: float,
    position: ref(point),
  };
  type elements = list(element);
  type key_state = {
    w: ref(bool),
    a: ref(bool),
    s: ref(bool),
    d: ref(bool),
  };

  type state = {
    ctx: GL_extern.ctx,
    shader_program: ShaderProgram.t,
    elements: elements,
    camera_pos: ref(Matrix.M4.t),
    key_state: key_state,
  }

  let make = canvas: Belt.Result.t(state, string) => {
    let ctx = Canvas.getWebGLContext(canvas);
    let shader_program = ShaderProgram.make(ctx);

    let elements = Array.make(200, ())
      |> Array.mapi((i, _) => {
        let rotation = float_of_int(i) *. 0.1;
        let x = float_of_int(i / 5) *. 1.0;
        let (y, z, scale) = switch (i mod 5) {
          | 0 => (1.0, 1.0, 0.2)
          | 1 => (0.5, 0.5, 0.1)
          | 2 => (0.0, 0.0, 0.1)
          | 3 => (-0.5, -0.5, 0.1)
          | 4 => (-1.0, -1.0, 0.2)
          | _ => (1.0, 1.0, 0.2)
        };
        {rotation: ref(rotation), scale: scale, position: ref((x, y, z))}
      })
      |> Array.to_list;
    Belt.Result.map(shader_program, shader_program => {
      {
        ctx,
        shader_program,
        elements,
        camera_pos: ref(Matrix.M4.identity() |> Matrix.M4.mul(Matrix.M4.translationZ(-5.0))),
        key_state: {
          w: ref(false),
          a: ref(false),
          s: ref(false),
          d: ref(false),
        }
      }
    })
  };

  /** Update the game state */
  let update = (state: state, time: float, dt: float) => {
    let now = time *. 0.001;
    List.iteri((i, el) => {
      el.rotation := mod_float(el.rotation^ +. dt *. 0.001, Js.Math._PI *. 2.);
    }, state.elements);

    let cameraMoveX = 0.0
      +. (state.key_state.a^ ? 0.006 *. dt : 0.0)
      +. (state.key_state.d^ ? -0.006 *. dt : 0.0);

    let cameraMoveZ = 0.0
      +. (state.key_state.w^ ? 0.01 *. dt : 0.0)
      +. (state.key_state.s^ ? -0.01 *. dt : 0.0);

    state.camera_pos :=
      Matrix.M4.identity()
      |> Matrix.M4.mul(Matrix.M4.translation(cameraMoveX, 0.0, cameraMoveZ))
      |> Matrix.M4.mul(state.camera_pos^);
    ShaderProgram.setModelViewMatrix(state.shader_program, state.camera_pos^);
  };

  /** Draw everything */
  let draw = (state: state) => {
    open GL_extern;
    let { ctx, shader_program } = state;

    clearColor(ctx, 0.1, 0.35, 0.5, 0.7);
    clearDepth(ctx, 1.0);
    clear(ctx, c_COLOR_BUFFER_BIT lor c_DEPTH_BUFFER_BIT);
    enable(ctx, c_DEPTH_TEST);
    depthFunc(ctx, c_LEQUAL);

    List.iteri((i, el) => {
      let rotation = el.rotation^;
      let (x, y, z) = el.position^;
      let transformMatrix =
        Matrix.M4.identity()
        |> Matrix.M4.mul(Matrix.M4.rotateX(rotation))
        |> Matrix.M4.mul(Matrix.M4.translation(x, y, z))
        |> Matrix.M4.mul(Matrix.M4.scale(el.scale, el.scale, el.scale));
      ShaderProgram.setTransform(state.shader_program, transformMatrix);

      ShaderProgram.draw(ctx, shader_program);
    }, state.elements);

  };

  let run = () => {
    let canvas = Canvas.find();
    let game = make(canvas);
    let last_time = ref(0.0);

    switch (game) {
    | Belt.Result.Error(err) => Js.log2("Error starting game", err)
    | Belt.Result.Ok(game) =>
      Document.addEventListener(document, "click", (_) => {
        Canvas.requestPointerLock(canvas);
      });

      Document.addEventListener(document, "keydown", (e) => {
        switch (Document.getKey(e)) {
          | "w" => game.key_state.w := true
          | "a" => game.key_state.a := true
          | "s" => game.key_state.s := true
          | "d" => game.key_state.d := true
          | _ => ()
        }
      });

      Document.addEventListener(document, "keyup", (e) => {
        switch (Document.getKey(e)) {
          | "w" => game.key_state.w := false
          | "a" => game.key_state.a := false
          | "s" => game.key_state.s := false
          | "d" => game.key_state.d := false
          | _ => ()
        }
      });

      Document.addEventListener(document, "mousemove", (e) => {
        let dx = Document.getMovementX(e);
        let dy = Document.getMovementY(e);

        game.camera_pos := Matrix.M4.identity()
          |> Matrix.M4.mul(Matrix.M4.rotateY(dx *. -0.001))
          |> Matrix.M4.mul(Matrix.M4.rotateX(dy *. -0.001))
          |> Matrix.M4.mul(game.camera_pos^);
        ShaderProgram.setModelViewMatrix(game.shader_program, game.camera_pos^);
      });

      let rec tick = time => {
        let dt = time == 0.0 ? 0.0 : time -. last_time^;
        last_time := time;
        update(game, time, dt);
        draw(game);
        requestAnimationFrame(window, tick);
      };
      tick(0.0);
    };
  };
}

let _ = Game.run();
