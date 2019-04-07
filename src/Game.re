module Canvas = {
  type t;
  let find = (): t => [%bs.raw {|document.getElementById("canvas")|}];
  [@bs.send] external getContext: (t, string) => GL_extern.ctx = "getContext";
  let getWebGLContext = (t: t): GL_extern.ctx => getContext(t, "webgl");
};

type window;
[@bs.val] external window: window = "window";
[@bs.send]
external requestAnimationFrame: (window, float => unit) => unit =
  "requestAnimationFrame";

module Game = {
  type element = {
    rotation: ref(float),
  };
  type elements = list(element);

  type state = {
    ctx: GL_extern.ctx,
    shader_program: ShaderProgram.t,
    elements: elements,
  }

  let make = (): Belt.Result.t(state, string) => {
    let ctx = Canvas.getWebGLContext(Canvas.find());
    let shader_program = ShaderProgram.make(ctx);

    Belt.Result.map(shader_program, shader_program => {
      {
        ctx,
        shader_program,
        elements: [
          {rotation: ref(0.)},
        ],
      }
    })
  };

  /** Update the game state */
  let update = (state: state, time: float) => {
    let now = time *. 0.001;
    List.hd(state.elements).rotation := Js.Math.sin(now);

    let rotation = List.hd(state.elements).rotation^;
    let modelViewMatrix =
      Matrix.M4.translation(0., 0., -6.)
      |> Matrix.M4.mul(Matrix.M4.rotateZ(rotation))
      |> Matrix.M4.mul(Matrix.M4.rotateY(rotation));
    ShaderProgram.setModelViewMatrix(state.shader_program, modelViewMatrix);
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

    ShaderProgram.uploadColors(shader_program);
    ShaderProgram.draw(ctx, shader_program);
  };

  let run = () => {
    let game = make();

    switch (game) {
    | Belt.Result.Error(err) => Js.log2("Error starting game", err)
    | Belt.Result.Ok(game) =>
      let rec tick = time => {
        update(game, time);
        draw(game);
        requestAnimationFrame(window, tick);
      };
      tick(0.0);
    };
  };
}

let _ = Game.run();
