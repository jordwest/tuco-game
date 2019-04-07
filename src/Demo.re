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
    projectionMatrixLoc: GL_extern.uniformLocation,
    modelViewMatrixLoc: GL_extern.uniformLocation,
    colorBuffer: GL_extern.buffer,
  };

  let uploadColors = (ctx, program, v) => {
    open GL_extern;
    let wrap = a => Js.Math.sin(a);

    let colors = [|
      wrap(v +. 1.0),
      wrap(v +. 0.0),
      wrap(v +. 0.0),
      1.0,
      wrap(v +. 0.0),
      wrap(v +. 1.0),
      wrap(v +. 0.0),
      1.0,
      wrap(v +. 0.0),
      wrap(v +. 0.0),
      wrap(v +. 1.0),
      1.0,
      wrap(v +. 1.0),
      wrap(v +. 1.0),
      wrap(v +. 0.0),
      1.0,
    |];
    bindBuffer(ctx, c_ARRAY_BUFFER, program.colorBuffer);
    bufferData(
      ctx,
      c_ARRAY_BUFFER,
      Js.TypedArray2.Float32Array.make(colors),
      c_STATIC_DRAW,
    );
  };

  let make = ctx => {
    open GL_extern;
    let program = GL.makeProgram(ctx, vertShaderSrc, fragShaderSrc);
    let positionBuffer = createBuffer(ctx);
    let colorBuffer = createBuffer(ctx);
    let positions = [|(-1.0), 1.0, 1.0, 1.0, (-1.0), (-1.0), 1.0, (-1.0)|];

    Belt.Result.map(
      program,
      program => {
        clearColor(ctx, 0.0, 0.0, 0.0, 1.0);
        clearDepth(ctx, 1.0);
        enable(ctx, c_DEPTH_TEST);
        depthFunc(ctx, c_LEQUAL);

        clear(ctx, c_COLOR_BUFFER_BIT lor c_DEPTH_BUFFER_BIT);

        /////// Camera ////////
        let fov = 45. *. Js.Math._PI /. 180.;
        let canvas = getCanvas(ctx);
        let aspect = getClientWidth(canvas) /. getClientHeight(canvas);
        let zNear = 0.1;
        let zFar = 100.;
        let projectionMatrix = Matrix.M4.perspective(fov, aspect, zNear, zFar);
        let modelViewMatrix = Matrix.M4.translation(-0., 0., -6.);

        let vertexPositionAttrib =
          getAttribLocation(ctx, program, "aVertexPosition");
        let vertexColorAttrib =
          getAttribLocation(ctx, program, "aVertexColor");
        let projectionMatrixLoc =
          getUniformLocation(ctx, program, "uProjectionMatrix");
        let modelViewMatrixLoc =
          getUniformLocation(ctx, program, "uModelViewMatrix");

        /////// Position Buffer //////
        bindBuffer(ctx, c_ARRAY_BUFFER, positionBuffer);
        bufferData(
          ctx,
          c_ARRAY_BUFFER,
          Js.TypedArray2.Float32Array.make(positions),
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

        let p = {
          program,
          vertexPositionAttrib,
          vertexColorAttrib,
          projectionMatrixLoc,
          modelViewMatrixLoc,
          colorBuffer,
        };

        /////// Color buffer ////////
        uploadColors(ctx, p, 0.1);

        vertexAttribPointer(ctx, vertexColorAttrib, 4, c_FLOAT, false, 0, 0);
        enableVertexAttribArray(ctx, vertexColorAttrib);

        useProgram(ctx, program);

        uniformMatrix4fv(
          ctx,
          projectionMatrixLoc,
          false,
          Js.TypedArray2.Float32Array.make(projectionMatrix),
        );
        uniformMatrix4fv(
          ctx,
          modelViewMatrixLoc,
          false,
          Js.TypedArray2.Float32Array.make(modelViewMatrix)
        );

        p;
      },
    );
  };

  let draw = ctx => {
    open GL_extern;
    drawArrays(ctx, c_TRIANGLE_STRIP, 0, 4);
  };
};

type window;
[@bs.val] external window: window = "window";
[@bs.send]
external requestAnimationFrame: (window, int => unit) => unit =
  "requestAnimationFrame";

let start = () => {
  open GL_extern;
  module Result = Belt.Result;

  let ctx = Canvas.getWebGLContext(Canvas.find());

  let demoProgram = DemoProgram.make(ctx);
  let clear = () => {
    clearColor(ctx, 0.0, 0.0, 0.0, 1.0);
    clear(ctx, c_COLOR_BUFFER_BIT lor c_DEPTH_BUFFER_BIT);
  };

  switch (demoProgram) {
  | Result.Error(err) => Js.log(err)
  | Result.Ok(program) =>
    let col = ref(0.0);
    let rec draw = _time => {
      col := col^ +. 0.01;
      clear();
      DemoProgram.draw(ctx);
      DemoProgram.uploadColors(ctx, program, col^);
      requestAnimationFrame(window, draw);
    };
    draw(0);
  };
};

let _ = start();