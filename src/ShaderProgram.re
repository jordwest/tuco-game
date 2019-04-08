[@bs.module "./shader.vert"] external vertShaderSrc: string = "default";
[@bs.module "./shader.frag"] external fragShaderSrc: string = "default";

type t = {
  ctx: GL_extern.ctx,
  program: GL_extern.program,
  vertexPositionAttrib: GL_extern.attribLocation,
  vertexColorAttrib: GL_extern.attribLocation,
  projectionMatrixLoc: GL_extern.uniformLocation,
  modelViewMatrixLoc: GL_extern.uniformLocation,
  transformLoc: GL_extern.uniformLocation,
  colorBuffer: GL_extern.buffer,
  indexBuffer: GL_extern.buffer,
};

let setVertexColor = (t, arr) => {
  open GL_extern;

  bindBuffer(t.ctx, c_ARRAY_BUFFER, t.colorBuffer);
  bufferData(
    t.ctx,
    c_ARRAY_BUFFER,
    arr,
    c_STATIC_DRAW,
  );
}

let uploadColors = (t) => {
  let faceColors = [|
    [| 1.0, 0.75, 0.0, 1.0 |],    // Front face: orange
    [| 0.95, 0.26, 0.21, 1.0 |],  // Back face: yellow
    [| 1.0, 0.84, 0.0, 1.0 |],    // Top face: yellow
    [| 0.54, 0.76, 0.29, 1.0 |],  // Bottom face: green
    [| 0.13, 0.58, 0.95, 1.0 |],  // Right face: blue
    [| 1.0, 0.0, 1.0, 1.0 |],     // Left face: purple
  |];
  let colors = Array.fold_left((colors, color) => {
    // Repeat each color 4 times for each vertex of the face
    Array.concat([colors, color, color, color, color])
  }, [||], faceColors);

  setVertexColor(t, Js.TypedArray2.Float32Array.make(colors));
};

let setTransform = (t, matrix) => {
  GL_extern.useProgram(t.ctx, t.program);
  GL_extern.uniformMatrix4fv(
    t.ctx,
    t.transformLoc,
    false,
    matrix,
  );
}

let setProjectionMatrix = (t, matrix) => {
  GL_extern.useProgram(t.ctx, t.program);
  GL_extern.uniformMatrix4fv(
    t.ctx,
    t.projectionMatrixLoc,
    false,
    matrix,
  );
}

let setModelViewMatrix = (t, matrix) => {
  GL_extern.useProgram(t.ctx, t.program);
  GL_extern.uniformMatrix4fv(
    t.ctx,
    t.modelViewMatrixLoc,
    false,
    matrix,
  );
}

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
    let modelViewMatrix = Matrix.M4.identity();

    let vertexPositionAttrib =
      getAttribLocation(ctx, program, "aVertexPosition");
    let vertexColorAttrib =
      getAttribLocation(ctx, program, "aVertexColor");
    let projectionMatrixLoc =
      getUniformLocation(ctx, program, "uProjectionMatrix");
    let modelViewMatrixLoc =
      getUniformLocation(ctx, program, "uModelViewMatrix");
    let transformLoc =
      getUniformLocation(ctx, program, "uTransform");

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
      ctx,
      program,
      vertexPositionAttrib,
      vertexColorAttrib,
      projectionMatrixLoc,
      modelViewMatrixLoc,
      transformLoc,
      colorBuffer,
      indexBuffer,
    };

    /////// Color buffer ////////
    uploadColors(p);
    bindBuffer(ctx, c_ARRAY_BUFFER, colorBuffer);
    vertexAttribPointer(ctx, vertexColorAttrib, 4, c_FLOAT, false, 0, 0);
    enableVertexAttribArray(ctx, vertexColorAttrib);

    useProgram(ctx, program);

    setProjectionMatrix(p, projectionMatrix)
    setModelViewMatrix(p, modelViewMatrix)
    setTransform(p, Matrix.M4.identity())
    p;
    },
  );
};

let draw = (ctx, p) => {
  open GL_extern;

  bindBuffer(ctx, c_ELEMENT_ARRAY_BUFFER, p.indexBuffer);
  let vertexCount = 36;
  let offset = 0;
  drawElements(ctx, c_TRIANGLES, vertexCount, c_UNSIGNED_SHORT, offset);
};