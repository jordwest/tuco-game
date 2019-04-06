module M4 {
  type t = array(float);
  exception WrongInput;
  let dimension = 4;
  let size = dimension * dimension;

  let make = items => {
    Array.length(items) < size
      ? raise(WrongInput)
      : items;
  };

  let identity = _ => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.,
  |]);

  let translate = (x, y, z) => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    x,  y,  z,  1.,
  |]);

  let translateX = x => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    x,  0., 0., 1.,
  |]);

  let translateY = y => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., y,  0., 1.,
  |]);

  let translateZ = z => make([|
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., z,  1.,
  |]);

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

  let transpose = m => {
    let t = Array.make_float(size);
    Array.iteri((i, _) => {
      let col = i / dimension;
      let row = i mod dimension;
      t[dimension * col + row] = m[dimension * row + col];
    }, m);
    t;
  };

  let mul = (a, b) => {
    let c = Array.make_float(size);
    Array.iteri((i, _) => {
      let col = i / dimension;
      let row = i mod dimension;
      let v =  a[dimension * row + 0] *. b[dimension * 0 + col]
            +. a[dimension * row + 1] *. b[dimension * 1 + col]
            +. a[dimension * row + 2] *. b[dimension * 2 + col]
            +. a[dimension * row + 3] *. b[dimension * 3 + col];
      c[dimension * row + col] = v;
    }, a);
    c;
  };

  let eq = (a, b) => {
    let rec eq_rec = i => {
      switch (i) {
        | i when i >= size => true
        | i when a[i] != b[i] => false
        | i => eq_rec(i + 1)
      }
    }
    eq_rec(0);
  };

  let print = (m: t) => {
    Js.log(
      Js.Array.joinWith(
        "",
        Array.mapi((i, v) =>
          Js.Float.toString(v) ++
          ((i + 1) mod dimension == 0
            ? "\n"
            : "\t"),
          m,
        )
      )
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
