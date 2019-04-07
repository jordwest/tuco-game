// Todo:
// 1. Add an ability to mutate matrices (to avoid allocation of new ones)
// 2. Use Float32Array instead of [||] to represent matrices to avoid a
//    conversion when we pass a matrix into shader
module M4 {
  module F32Array = Js.TypedArray2.Float32Array;
  type t = F32Array.t;
  exception WrongInput;
  let dimension = 4;
  let size = dimension * dimension;

  let make = (items: array(float)) => {
    Array.length(items) < size
      ? raise(WrongInput)
      : F32Array.make(items);
  };

  let identity = _ => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.,
  |]);

  let transpose = m => {
    let result = F32Array.fromLength(size);
    F32Array.forEachi(m, (. _, i) => {
      let col = i / dimension;
      let row = i mod dimension;
      F32Array.unsafe_set(
        result,
        dimension * col + row,
        F32Array.unsafe_get(m, dimension * row + col),
      );
    });
    result;
  };

  let mul = (a, b) => {
    let result = F32Array.fromLength(size);
    F32Array.forEachi(a, (. _, i) => {
      let col = i / dimension;
      let row = i mod dimension;
      let v =  F32Array.unsafe_get(a, dimension * row + 0) *. F32Array.unsafe_get(b, dimension * 0 + col)
            +. F32Array.unsafe_get(a, dimension * row + 1) *. F32Array.unsafe_get(b, dimension * 1 + col)
            +. F32Array.unsafe_get(a, dimension * row + 2) *. F32Array.unsafe_get(b, dimension * 2 + col)
            +. F32Array.unsafe_get(a, dimension * row + 3) *. F32Array.unsafe_get(b, dimension * 3 + col);
      F32Array.unsafe_set(result, dimension * row + col, v);
    });
    result;
  };

  let eq = (a, b) => {
    let rec eq_rec = i => {
      switch (i) {
        | i when i >= size => true
        | i when F32Array.unsafe_get(a, i) != F32Array.unsafe_get(b, i) => false
        | i => eq_rec(i + 1)
      }
    }
    eq_rec(0);
  };

  let translation = (x, y, z) => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    x,  y,  z,  1.,
  |]);

  let translationX = x => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    x,  0., 0., 1.,
  |]);

  let translationY = y => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., y,  0., 1.,
  |]);

  let translationZ = z => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., z,  1.,
  |]);

  let translate = (m, x, y, z) => mul(m, translation(x, y, z));

  let scale = (x, y, z) => make([|
    x,  0., 0., 0.,
    0., y,  0., 0.,
    0., 0., z,  0.,
    0., 0., 0., 1.,
  |]);

  let scaleX = x => make([|
    x,  0., 0., 0.,
    0., 0., 0., 0.,
    0., 0., 0., 0.,
    0., 0., 0., 1.,
  |]);

  let scaleY = y => make([|
    0., 0., 0., 0.,
    0., y,  0., 0.,
    0., 0., 0., 0.,
    0., 0., 0., 1.,
  |]);

  let scaleZ = z => make([|
    0., 0., 0., 0.,
    0., 0., 0., 0.,
    0., 0., z,  0.,
    0., 0., 0., 1.,
  |]);

  let rotateX = a => {
    let c = Js.Math.cos(a);
    let s = Js.Math.sin(a);
    make([|
      1., 0., 0., 0.,
      0., c,-.s,  0.,
      0., s,  c,  0.,
      0., 0., 0., 1.,
    |]);
  };

  let rotateY = a => {
    let c = Js.Math.cos(a);
    let s = Js.Math.sin(a);
    make([|
        c,  0., s,  0.,
        0., 1., 0., 0.,
      -.s,  0., c,  0.,
        0., 0., 0., 1.,
    |]);
  };

  let rotateZ = a => {
    let c = Js.Math.cos(a);
    let s = Js.Math.sin(a);
    make([|
      c,-.s,  0., 0.,
      s,  c,  0., 0.,
      0., 0., 1., 0.,
      0., 0., 0., 1.,
    |]);
  };

  // Perspective explaind https://www.youtube.com/watch?v=mpTl003EXCY
  let perspective(fovy, aspect, near, far) {
    let f = 1. /. Js.Math.tan(fovy /. 2.);
    let nf = 1. /. (near -. far);
    make([|
      f /. aspect, 0., 0., 0.,
      0., f, 0., 0.,
      0., 0., (far +. near) *. nf, -1.,
      0., 0., 2. *. far *. near *. nf, 0.,
    |]);
  };

  let print = (m: t) => {
    Js.log(
      Js.TypedArray2.Float32Array.reducei(
        m,
        (. result, v, i) =>
          result ++ Js.Float.toString(v) ++
          ((i + 1) mod dimension == 0
            ? "\n"
            : "\t"),
        "",
      ),
    );
  };

  // let test = () => {
  //   Js.log(":: Make");
  //   make([|
  //     1.0, 1.0, 1.0, 1.0,
  //     1.0, 1.0, 1.0, 1.0,
  //     1.0, 1.0, 1.0, 1.0,
  //     1.0, 1.0, 1.0, 1.0,
  //   |])
  //     |> print;

  //   Js.log(":: Eq");
  //   eq(
  //     make([|
  //       1.0, 1.0, 1.0, 1.0,
  //       1.0, 1.0, 1.0, 1.0,
  //       1.0, 1.0, 1.0, 1.0,
  //       1.0, 1.0, 1.0, 1.0,
  //     |]),
  //     make([|
  //       1.0, 1.0, 1.0, 1.0,
  //       1.0, 1.0, 1.0, 1.0,
  //       1.0, 1.0, 1.0, 1.0,
  //       1.0, 1.0, 1.0, 1.0,
  //     |])
  //   )
  //     |> Js.log;

  //   Js.log(":: Mul1");
  //   mul(
  //     make([|
  //       0.5,  0.5,  0.0, 0.0,
  //       0.25, 0.25, 0.0, 0.0,
  //       0.0,  0.0,  1.0, 0.0,
  //       0.0,  0.0,  0.0, 1.0,
  //     |]),
  //     make([|
  //       0.5,   0.0,  0.5,  0.0,
  //       0.0,   1.0,  0.0,  0.0,
  //       0.25,  0.0,  0.25, 0.0,
  //       0.0,   0.0,  0.0,  1.0,
  //     |])
  //   )
  //     |> print;

  //   Js.log(":: Mul identity");
  //   mul(
  //     make([|
  //       0.5,  0.5,  0.0, 0.0,
  //       0.25, 0.25, 0.0, 0.0,
  //       0.0,  0.0,  1.0, 0.0,
  //       0.0,  0.0,  0.0, 1.0,
  //     |]),
  //     identity()
  //   )
  //     |> print;

  //   Js.log(":: transpose");

  //   make([|
  //     0.5,  0.5,  0.0, 0.0,
  //     0.25, 0.25, 0.0, 0.0,
  //     0.0,  0.0,  1.0, 0.0,
  //     0.0,  0.0,  0.0, 1.0,
  //   |]) |> transpose |> print;
  // }
};
