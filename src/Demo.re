module Float32Array = {
  type t;
  [@bs.new] external make: Js.Array.t(float) => t = "Float32Array";
};

module Canvas = {
  type t;
  let find = (): t => [%bs.raw {|document.getElementById("canvas")|}];
  [@bs.send] external getContext: (t, string) => GL_extern.ctx = "getContext";
  let getWebGLContext = (t: t): GL_extern.ctx => getContext(t, "webgl");
};

[@bs.module "./shader.vert"] external vertShaderSrc: string = "default";
[@bs.module "./shader.frag"] external fragShaderSrc: string = "default";

module DemoProgram = {
  type t = {
    program: GL_extern.program,
    vertexPositionAttrib: GL_extern.attribLocation,
    vertexColorAttrib: GL_extern.attribLocation,
  };

  let make = ctx => {
    open GL_extern;
    let program = GL.makeProgram(ctx, vertShaderSrc, fragShaderSrc);
    let positionBuffer = createBuffer(ctx);
    let colorBuffer = createBuffer(ctx);
    let positions = [|(-0.5), 0.5, 0.5, 0.5, (-0.5), (-0.5), 0.5, (-0.5)|];
    let colors = [|
      1.0,
      0.0,
      0.0,
      1.0,
      0.0,
      1.0,
      0.0,
      1.0,
      0.0,
      0.0,
      1.0,
      1.0,
      1.0,
      1.0,
      1.0,
      1.0,
    |];

    Belt.Result.map(
      program,
      program => {
        let vertexPositionAttrib =
          getAttribLocation(ctx, program, "aVertexPosition");
        let vertexColorAttrib =
          getAttribLocation(ctx, program, "aVertexColor");

        /////// Position Buffer //////
        bindBuffer(ctx, c_ARRAY_BUFFER, positionBuffer);
        bufferData(
          ctx,
          c_ARRAY_BUFFER,
          Float32Array.make(positions),
          c_STATIC_DRAW,
        );

        vertexAttribPointer(
          ctx,
          vertexPositionAttrib,
          2,
          c_FLOAT,
          false,
          0,
          0,
        );
        enableVertexAttribArray(ctx, vertexPositionAttrib);

        /////// Color buffer ////////
        bindBuffer(ctx, c_ARRAY_BUFFER, colorBuffer);
        bufferData(
          ctx,
          c_ARRAY_BUFFER,
          Float32Array.make(colors),
          c_STATIC_DRAW,
        );

        vertexAttribPointer(ctx, vertexColorAttrib, 4, c_FLOAT, false, 0, 0);
        enableVertexAttribArray(ctx, vertexColorAttrib);

        {program, vertexPositionAttrib, vertexColorAttrib};
      },
    );
  };

  let draw = (ctx, t) => {
    open GL_extern;

    useProgram(ctx, t.program);
    drawArrays(ctx, c_TRIANGLE_STRIP, 0, 4);
  };
};

let start = () => {
  open GL_extern;
  module Result = Belt.Result;

  let ctx = Canvas.getWebGLContext(Canvas.find());
  clearColor(ctx, 0.0, 0.0, 0.0, 1.0);
  clear(ctx, c_COLOR_BUFFER_BIT);

  let demoProgram = DemoProgram.make(ctx);
  switch (demoProgram) {
  | Result.Error(err) => Js.log(err)
  | Result.Ok(program) =>
    DemoProgram.draw(ctx, program);
    DemoProgram.draw(ctx, program);
    DemoProgram.draw(ctx, program);
    DemoProgram.draw(ctx, program);
  };
  ();
};

let _ = start();