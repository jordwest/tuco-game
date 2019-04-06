let loadShader = (gl, shaderType, source) => {
  open GL_extern;

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
  open GL_extern;
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