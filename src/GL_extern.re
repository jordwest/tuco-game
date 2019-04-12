/** Raw bindings to WebGL functions */

type canvas;
type ctx;
type shader;
type program;
type buffer;
type texture;
type uniformLocation;
type attribLocation;

let c_TEXTURE_2D: int = [%bs.raw {|3553|}];
let c_FLOAT: int = [%bs.raw {|5126|}];
let c_UNSIGNED_SHORT: int = [%bs.raw {|5123|}];
let c_ARRAY_BUFFER: int = [%bs.raw {|34962|}];
let c_ELEMENT_ARRAY_BUFFER: int = [%bs.raw {|34963|}];
let c_DEPTH_BUFFER_BIT: int = [%bs.raw {|256|}];
let c_COLOR_BUFFER_BIT: int = [%bs.raw {|16384|}];
let c_VERTEX_SHADER: int = [%bs.raw {|35633|}];
let c_FRAGMENT_SHADER: int = [%bs.raw {|35632|}];
let c_COMPILE_STATUS: int = [%bs.raw {|35713|}];
let c_LINK_STATUS: int = [%bs.raw {|35714|}];
let c_DEPTH_TEST: int = [%bs.raw {|2929|}];
let c_LEQUAL: int = [%bs.raw {|515|}];
let c_STATIC_DRAW: int = [%bs.raw {|35044|}];
let c_TRIANGLE_STRIP: int = [%bs.raw {|5|}];
let c_TRIANGLE_FAN: int = [%bs.raw {|6|}];
let c_TRIANGLES: int = [%bs.raw {|4|}];

let c_RGBA: int = [%bs.raw {|6408|}];
let c_UNSIGNED_BYTE: int = [%bs.raw {|5121|}];
let c_TEXTURE_WRAP_S: int = [%bs.raw {|10242|}];
let c_TEXTURE_WRAP_T: int = [%bs.raw {|10243|}];
let c_CLAMP_TO_EDGE: int = [%bs.raw {|33071|}];
let c_TEXTURE_MIN_FILTER: int = [%bs.raw {|10241|}];
let c_LINEAR: int = [%bs.raw {|9729|}];
let c_TEXTURE0: int = [%bs.raw {|33984|}];

[@bs.get] external getCanvas: ctx => canvas = "canvas";
[@bs.get] external getClientWidth: canvas => float = "clientWidth";
[@bs.get] external getClientHeight: canvas => float = "clientHeight";
[@bs.send] external enable: (ctx, int) => unit = "enable";
[@bs.send] external depthFunc: (ctx, int) => unit = "depthFunc";
[@bs.send]
external clearColor: (ctx, float, float, float, float) => unit = "clearColor";
[@bs.send] external clear: (ctx, int) => unit = "clear";
[@bs.send] external createShader: (ctx, int) => shader = "createShader";
[@bs.send] external createBuffer: ctx => buffer = "createBuffer";
[@bs.send] external createProgram: ctx => program = "createProgram";
[@bs.send] external bindBuffer: (ctx, int, buffer) => unit = "bindBuffer";
[@bs.send]
external vertexAttribPointer:
  (ctx, attribLocation, int, int, bool, int, int) => unit =
  "vertexAttribPointer";
[@bs.send]
external enableVertexAttribArray: (ctx, attribLocation) => unit =
  "enableVertexAttribArray";
[@bs.send] external bufferData: (ctx, int, 'a, int) => unit = "bufferData";
[@bs.send] external drawArrays: (ctx, int, int, int) => unit = "drawArrays";
[@bs.send] external drawElements: (ctx, int, int, int, int) => unit = "drawElements";
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
external getShaderParameter: (ctx, shader, int) => 'a = "getShaderParameter";
[@bs.send]
external getProgramParameter: (ctx, program, int) => 'a =
  "getProgramParameter";
[@bs.send]
external getUniformLocation: (ctx, program, string) => uniformLocation =
  "getUniformLocation";
[@bs.send]
external getAttribLocation: (ctx, program, string) => attribLocation =
  "getAttribLocation";
[@bs.send] external clearDepth: (ctx, float) => unit = "clearDepth";
[@bs.send]
external uniformMatrix4fv:
  (ctx, uniformLocation, bool, Js.TypedArray2.Float32Array.t) => unit =
  "uniformMatrix4fv";
[@bs.send] external createTexture: (ctx) => texture = "createTexture";
[@bs.send] external bindTexture: (ctx, int, texture) => unit = "bindTexture";
[@bs.send] external texParameteri: (ctx, int, int, int) => unit = "texParameteri";
[@bs.send] external generateMipmap: (ctx, int) => unit = "";

module Image = {
  type t;
  [@bs.new] external make: unit => t = "Image";
  [@bs.set] external setOnload: (t, (unit => unit)) => unit = "onload";
  [@bs.set] external setSrc: (t, string) => unit = "src";
}

[@bs.send] external texImage2D_u8: (ctx, int, int, int, int, int, int, int, int, Js.TypedArray2.Uint8Array.t) => unit = "";
[@bs.send] external texImage2D_image: (ctx, int, int, int, int, int, Image.t) => unit = "";