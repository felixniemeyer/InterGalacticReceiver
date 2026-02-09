//cell 002
//by Grit Kit
//IG: gritkitvisuals
//https://gritschuster.de/

/* Simplified version by Gabor */
shape(50, 0.4, () => (Math.sin(time) + 1.5) * 0.05)
  .sub(shape(50, () => (Math.sin(time * 0.5) + 1.0) * 0.12, 0.2).modulateScale(o1, 0.5))
  .add(voronoi(60, 1, 5).mult(shape(50, 0.4, 0.2).invert(1)))
  .mult(osc(23, -0.05, 1000)
    .brightness(0.3)
    .saturate(0.6)
    .kaleid(50))
  .modulateScale(o1, 0.2)
  .rotate(1, 0.1)
  .modulate(o1, 0.05)
  .scale(1, 1, 1.5)
  .out()

noise(5, 0.1).add(solid(1, 1, 1), 0.5).rotate(1, -0.05).out(o1)


/* Original
shape(50, 0.4, () => (Math.sin(time) + 1.5) * 0.05)
  .sub(shape(50, () => (Math.sin(time * 0.5) + 1.0) * 0.12, 0.2).modulateScale(o1, 0.5))
  .add(o3)
  .mult(osc(23, -0.05, 1000)
    .brightness(0.3)
    .saturate(0.6)
    .kaleid(50))
  .modulateScale(o1, 0.2)
  .rotate(1, 0.1)
  .modulateScale(o2, 0.1)
  .modulate(o1, 0.05)
  .scale(1, 1, 1.5)
  .out(o0)

noise(5, 0.1).add(solid(1, 1, 1), 0.5).rotate(1, -0.05).out(o1)

noise(500, 1).contrast(0.8).brightness(0.3)
  .sub(shape(50, 0.4, 0.2).invert(1))
  .out(o2)

voronoi(60, 1, 5)
  .mult(shape(50, 0.35, 0.1))
  .out(o3)

render(o0)
*/
