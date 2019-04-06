module Float32Array = {
  type t;
  [@bs.new] external make: Js.Array.t(float) => t = "Float32Array";
};

module GL = {
  type ctx;
  type glConst;
  type shader;
  type program;
  type buffer;
  type uniformLocation;
  type attribLocation;

  let c_TEXTURE_2D: glConst = [%bs.raw {|3553|}];
  let c_FLOAT: glConst = [%bs.raw {|5126|}];
  let c_ARRAY_BUFFER: glConst = [%bs.raw {|34962|}];
  let c_DEPTH_BUFFER_BIT: glConst = [%bs.raw {|256|}];
  let c_COLOR_BUFFER_BIT: glConst = [%bs.raw {|16384|}];
  let c_VERTEX_SHADER: glConst = [%bs.raw {|35633|}];
  let c_FRAGMENT_SHADER: glConst = [%bs.raw {|35632|}];
  let c_COMPILE_STATUS: glConst = [%bs.raw {|35713|}];
  let c_LINK_STATUS: glConst = [%bs.raw {|35714|}];
  let c_DEPTH_TEST: glConst = [%bs.raw {|2929|}];
  let c_LEQUAL: glConst = [%bs.raw {|515|}];
  let c_STATIC_DRAW: glConst = [%bs.raw {|35044|}];
  let c_TRIANGLE_STRIP: glConst = [%bs.raw {|5|}];
  let c_TRIANGLE_FAN: glConst = [%bs.raw {|6|}];
  let c_TRIANGLES: glConst = [%bs.raw {|4|}];

  [@bs.send]
  external clearColor: (ctx, float, float, float, float) => unit =
    "clearColor";
  [@bs.send] external clear: (ctx, glConst) => unit = "clear";
  [@bs.send] external createShader: (ctx, glConst) => shader = "createShader";
  [@bs.send] external createBuffer: ctx => buffer = "createBuffer";
  [@bs.send] external createProgram: ctx => program = "createProgram";
  [@bs.send]
  external bindBuffer: (ctx, glConst, buffer) => unit = "bindBuffer";
  [@bs.send]
  external vertexAttribPointer:
    (ctx, attribLocation, int, glConst, bool, int, int) => unit =
    "vertexAttribPointer";
  [@bs.send]
  external enableVertexAttribArray: (ctx, attribLocation) => unit =
    "enableVertexAttribArray";
  [@bs.send]
  external bufferData: (ctx, glConst, 'a, glConst) => unit = "bufferData";
  [@bs.send]
  external drawArrays: (ctx, glConst, int, int) => unit = "drawArrays";
  [@bs.send]
  external shaderSource: (ctx, shader, string) => unit = "shaderSource";
  [@bs.send] external compileShader: (ctx, shader) => unit = "compileShader";
  [@bs.send]
  external attachShader: (ctx, program, shader) => unit = "attachShader";
  [@bs.send] external useProgram: (ctx, program) => unit = "useProgram";
  [@bs.send] external deleteShader: (ctx, shader) => unit = "deleteShader";
  [@bs.send] external linkProgram: (ctx, program) => unit = "linkProgram";
  [@bs.send]
  external getShaderInfoLog: (ctx, shader) => string = "getShaderInfoLog";
  [@bs.send]
  external getProgramInfoLog: (ctx, program) => string = "getProgramInfoLog";
  [@bs.send]
  external getShaderParameter: (ctx, shader, glConst) => 'a =
    "getShaderParameter";
  [@bs.send]
  external getProgramParameter: (ctx, program, glConst) => 'a =
    "getProgramParameter";
  [@bs.send]
  external getUniformLocation: (ctx, program, string) => uniformLocation =
    "getUniformLocation";
  [@bs.send]
  external getAttribLocation: (ctx, program, string) => attribLocation =
    "getAttribLocation";
  [@bs.send] external clearDepth: (ctx, float) => unit = "clearDepth";
};

module Canvas = {
  type t;
  let find = (): t => [%bs.raw {|document.getElementById("canvas")|}];
  [@bs.send] external getContext: (t, string) => GL.ctx = "getContext";
  let getWebGLContext = (t: t): GL.ctx => getContext(t, "webgl");
};

[@bs.module "./shader.vert"] external vertShaderSrc: string = "default";
[@bs.module "./shader.frag"] external fragShaderSrc: string = "default";

let loadShader = (gl, shaderType, source) => {
  open GL;

  let shader = createShader(gl, shaderType);
  shaderSource(gl, shader, source);
  compileShader(gl, shader);

  getShaderParameter(gl, shader, c_COMPILE_STATUS)
    ? Belt.Result.Ok(shader)
    : {
      let error = getShaderInfoLog(gl, shader);
      deleteShader(gl, shader);
      Belt.Result.Error(error);
    };
};

let makeProgram = (gl, vertSource, fragSource) => {
  open GL;
  module Result = Belt.Result;

  let vertResult = loadShader(gl, c_VERTEX_SHADER, vertSource);
  let fragResult = loadShader(gl, c_FRAGMENT_SHADER, fragSource);

  switch (vertResult, fragResult) {
  // One of the shaders failed to compile 👎
  | (Result.Error(err), _) =>
    Result.Error("Could not compile vertex shader: " ++ err)

  | (_, Result.Error(err)) =>
    Result.Error("Could not compile fragment shader: " ++ err)

  // Both shaders compiled ok 👌
  | (Ok(vert), Ok(frag)) =>
    let program = gl->createProgram;
    attachShader(gl, program, vert);
    attachShader(gl, program, frag);
    linkProgram(gl, program);
    if (getProgramParameter(gl, program, c_LINK_STATUS)) {
      Result.Ok(program);
    } else {
      let info = gl->getProgramInfoLog(program);
      Result.Error("Unable to link shader program: " ++ info);
    };
  };
};

let start = () => {
  open GL;
  module Result = Belt.Result;

  let ctx = Canvas.getWebGLContext(Canvas.find());
  clearColor(ctx, 0.1, 0.0, 0.0, 1.0);
  clear(ctx, c_COLOR_BUFFER_BIT);
  Js.log(vertShaderSrc);
  Js.log(fragShaderSrc);
  let program = makeProgram(ctx, vertShaderSrc, fragShaderSrc);
  switch (program) {
  | Result.Error(err) => Js.log(err)
  | Result.Ok(program) =>
    let positionBuffer = createBuffer(ctx);
    let positions = [|(-0.5), 0.5, 0.5, 0.5, (-0.5), (-0.5), 0.5, (-0.5)|];
    let vertexAttrib = getAttribLocation(ctx, program, "aVertexPosition");

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