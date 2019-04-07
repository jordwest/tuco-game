module Canvas = {
  type t;
  let find = (): t => [%bs.raw {|document.getElementById("canvas")|}];
  [@bs.send] external getContext: (t, string) => GL_extern.ctx = "getContext";
  let getWebGLContext = (t: t): GL_extern.ctx => getContext(t, "webgl");
};

[@bs.module "./shader.vert"] external vertShaderSrc: string = "default";
[@bs.module "./shader.frag"] external fragShaderSrc: string = "default";

module ShaderProgram = {
  type element = {
    rotation: ref(float),
  };
  type elements = list(element);
  type t = {
    program: GL_extern.program,
    elements: elements,
    vertexPositionAttrib: GL_extern.attribLocation,
    vertexColorAttrib: GL_extern.attribLocation,
    projectionMatrixLoc: GL_extern.uniformLocation,
    modelViewMatrixLoc: GL_extern.uniformLocation,
    colorBuffer: GL_extern.buffer,
    indexBuffer: GL_extern.buffer,
  };

  let uploadColors = (ctx, program, v) => {
    open GL_extern;
    let faceColors = [|
      [| 1.0, 1.0, 1.0, 1.0 |], // Front face: white
      [| 1.0, 0.0, 0.0, 1.0 |], // Back face: red
      [| 0.0, 1.0, 0.0, 1.0 |], // Top face: green
      [| 0.0, 0.0, 1.0, 1.0 |], // Bottom face: blue
      [| 1.0, 1.0, 0.0, 1.0 |], // Right face: yellow
      [| 1.0, 0.0, 1.0, 1.0 |], // Left face: purple
    |];
    let colors = Array.fold_left((colors, color) => {
      // Repeat each color 4 times for each vertex of the face
      Array.concat([colors, color, color, color, color])
    }, [||], faceColors);

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
    let indexBuffer = createBuffer(ctx);
    let colorBuffer = createBuffer(ctx);
    // let positions = [|(-1.0), 1.0, 1.0, 1.0, (-1.0), (-1.0), 1.0, (-1.0)|];
    let positions = [|
      // Front face
      -1.0, -1.0, 1.0,
       1.0, -1.0, 1.0,
       1.0,  1.0, 1.0,
      -1.0,  1.0, 1.0,
      // Back face
      -1.0, -1.0, -1.0,
       1.0, -1.0, -1.0,
       1.0,  1.0, -1.0,
      -1.0,  1.0, -1.0,
      // Top face
      -1.0,  1.0, -1.0,
      -1.0,  1.0,  1.0,
       1.0,  1.0,  1.0,
       1.0,  1.0, -1.0,
      // Bottom face
      -1.0, -1.0,  1.0,
      -1.0, -1.0, -1.0,
       1.0, -1.0, -1.0,
       1.0, -1.0,  1.0,
      // Right face
       1.0, -1.0, -1.0,
       1.0, -1.0,  1.0,
       1.0,  1.0,  1.0,
       1.0,  1.0, -1.0,
      // Left face
      -1.0, -1.0, -1.0,
      -1.0, -1.0,  1.0,
      -1.0,  1.0,  1.0,
      -1.0,  1.0, -1.0,
    |];

    Belt.Result.map(
      program,
      program => {
        /////// Camera ////////
        let fov = 45. *. Js.Math._PI /. 180.;
        let canvas = getCanvas(ctx);
        let aspect = getClientWidth(canvas) /. getClientHeight(canvas);
        let zNear = 0.1;
        let zFar = 100.;
        let projectionMatrix = Matrix.M4.perspective(fov, aspect, zNear, zFar);
        let modelViewMatrix = Matrix.M4.mul(
          Matrix.M4.translation(0., 0., -6.),
          Matrix.M4.rotateZ(0.),
        );

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
        vertexAttribPointer(ctx, vertexPositionAttrib, 3, c_FLOAT, false, 0, 0);
        enableVertexAttribArray(ctx, vertexPositionAttrib);

        bindBuffer(ctx, c_ELEMENT_ARRAY_BUFFER, indexBuffer);
        let indices = [|
          0, 1, 2,      0, 2, 3,    // front
          4, 5, 6,      4, 6, 7,    // back
          8, 9, 10,     8, 10, 11,  // top
          12, 13, 14,   12, 14, 15, // bottom
          16, 17, 18,   16, 18, 19, // right
          20, 21, 22,   20, 22, 23, // left
        |];
        bufferData(
          ctx,
          c_ELEMENT_ARRAY_BUFFER,
          Js.TypedArray2.Uint16Array.make(indices),
          c_STATIC_DRAW,
        );

        let p = {
          program,
          elements: [
            {rotation: ref(0.)},
          ],
          vertexPositionAttrib,
          vertexColorAttrib,
          projectionMatrixLoc,
          modelViewMatrixLoc,
          colorBuffer,
          indexBuffer,
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
          projectionMatrix,
        );
        uniformMatrix4fv(
          ctx,
          modelViewMatrixLoc,
          false,
          modelViewMatrix,
        );

        p;
      },
    );
  };

  let draw = (ctx, p) => {
    open GL_extern;

    let rotation = List.hd(p.elements).rotation^;
    let modelViewMatrix =
      Matrix.M4.translation(0., 0., -6.)
      |> Matrix.M4.mul(Matrix.M4.rotateZ(rotation))
      |> Matrix.M4.mul(Matrix.M4.rotateY(rotation));
    uniformMatrix4fv(
      ctx,
      p.modelViewMatrixLoc,
      false,
      modelViewMatrix,
    );

    bindBuffer(ctx, c_ELEMENT_ARRAY_BUFFER, p.indexBuffer);
    let vertexCount = 36;
    let offset = 0;
    drawElements(ctx, c_TRIANGLES, vertexCount, c_UNSIGNED_SHORT, offset);
  };
};

module Game = {
  type state = {
    ctx: GL_extern.ctx,
    shader_program: ShaderProgram.t,
  }

  let make = (): Belt.Result.t(state, string) => {
    let ctx = Canvas.getWebGLContext(Canvas.find());
    let shader_program = ShaderProgram.make(ctx);

    Belt.Result.map(shader_program, shader_program => {
      {
        ctx,
        shader_program,
      }
    })
  };

  /** Update the game state */
  let update = (state: state, time: float) => {
    ();
  };

  /** Draw everything */
  let draw = (state: state, time: float) => {
    open GL_extern;
    let now = time *. 0.001;
    let { ctx, shader_program } = state;

    clearColor(ctx, 0.1, 0.35, 0.5, 0.7);
    clearDepth(ctx, 1.0);
    clear(ctx, c_COLOR_BUFFER_BIT lor c_DEPTH_BUFFER_BIT);
    enable(ctx, c_DEPTH_TEST);
    depthFunc(ctx, c_LEQUAL);

    ShaderProgram.draw(ctx, shader_program);
    ShaderProgram.uploadColors(ctx, shader_program, now);
    List.hd(shader_program.elements).rotation := Js.Math.sin(now);
  };
}

type window;
[@bs.val] external window: window = "window";
[@bs.send]
external requestAnimationFrame: (window, float => unit) => unit =
  "requestAnimationFrame";

let start = () => {
  let game = Game.make();

  switch (game) {
  | Belt.Result.Error(err) => Js.log2("Error starting game", err)
  | Belt.Result.Ok(game) =>
    let rec tick = time => {
      Game.update(game, time);
      Game.draw(game, time);
      requestAnimationFrame(window, tick);
    };
    tick(0.0);
  };
};

let _ = start();
