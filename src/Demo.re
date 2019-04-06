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

let start = () => {
  open GL_extern;
  open GL;
  module Result = Belt.Result;

  let ctx = Canvas.getWebGLContext(Canvas.find());
  clearColor(ctx, 0.0, 0.0, 0.0, 1.0);
  clear(ctx, c_COLOR_BUFFER_BIT);

  let program = makeProgram(ctx, vertShaderSrc, fragShaderSrc);
  switch (program) {
  | Result.Error(err) => Js.log(err)
  | Result.Ok(program) =>
    let positionBuffer = createBuffer(ctx);
    let positions = [|(-0.5), 0.5, 0.5, 0.5, (-0.5), (-0.5), 0.5, (-0.5)|];
    let colors = [|1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0|];
    let vertexAttrib = getAttribLocation(ctx, program, "aVertexPosition");
    let colorAttrib = getAttribLocation(ctx, program, "aVertexPosition");

    bindBuffer(ctx, c_ARRAY_BUFFER, positionBuffer);
    bufferData(
      ctx,
      c_ARRAY_BUFFER,
      Float32Array.make(positions),
      c_STATIC_DRAW,
    );

    bindBuffer(ctx, c_ARRAY_BUFFER, positionBuffer);
    vertexAttribPointer(ctx, vertexAttrib, 2, c_FLOAT, false, 0, 0);
    enableVertexAttribArray(ctx, vertexAttrib);

    useProgram(ctx, program);

    drawArrays(ctx, c_TRIANGLE_STRIP, 0, 4);
    ();
  };
  ();
};

let _ = start();