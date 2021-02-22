import {Entity, registerComponent} from 'aframe';
import {Vector3} from 'three';

const getChildEntityPosition = (childId: string): Vector3 => {
  const childEntity = document.querySelector(`#${childId}`);
  const childEntityPosition = childEntity?.object3D.position;
  const parentEntityPosition = (childEntity?.parentElement as Entity).object3D
    .position;
  return new Vector3().copy(childEntityPosition).add(parentEntityPosition);
};

const drawSpeakerEntity = (
  ctx: CanvasRenderingContext2D,
  x: number,
  y: number,
  radius: number,
  startAngle = 0,
  endAngle: number = 2 * Math.PI
) => {
  const circle = new Path2D();
  circle.arc(x, y, radius, startAngle, endAngle);
  ctx.fillStyle = '#FFFFFF';
  ctx.strokeStyle = '#FFFFFF';
  ctx.fill(circle);
  return circle;
};

const RADIUS = 30;
const circles = {
  'juror-a': [30, 120],
  'juror-c': [120, 30],
  'juror-b': [210, 30],
  'jury-foreman': [300, 120],
};

let currentCircle: string | null = null;

const setSpeakerAsActive = (speaker: string) => {
  const captionEl = document.querySelector('a-text#caption');
  const ambientCaptionEl = document.querySelector('a-text#ambientCaption');
  captionEl.setAttribute(
    'caption',
    `speaker: ${
      captionEl.getAttribute('caption').speaker
    }; activeTarget: ${speaker}; ambientCaption: ${
      captionEl.getAttribute('caption').ambientCaption
    }`
  );
  ambientCaptionEl.setAttribute(
    'caption',
    `speaker: ${
      ambientCaptionEl.getAttribute('caption').speaker
    }; activeTarget: ${speaker}; ambientCaption: ${
      ambientCaptionEl.getAttribute('caption').ambientCaption
    };`
  );
};

export const rotationReaderComponent = registerComponent('rotation-reader', {
  init: function () {
    const canvas: HTMLCanvasElement = document.getElementById(
      'global-setup'
    ) as HTMLCanvasElement;
    const ctx = canvas.getContext('2d')!;
    const [jax, jay] = circles['juror-a'];
    const [jbx, jby] = circles['juror-b'];
    const [jcx, jcy] = circles['juror-c'];
    const [jfx, jfy] = circles['jury-foreman'];
    const circlePaths: {[key: string]: Path2D} = {
      'juror-a': drawSpeakerEntity(ctx, jax, jay, RADIUS, 0, 2 * Math.PI),
      'juror-b': drawSpeakerEntity(ctx, jbx, jby, RADIUS, 0, 2 * Math.PI),
      'juror-c': drawSpeakerEntity(ctx, jcx, jcy, RADIUS, 0, 2 * Math.PI),
      'jury-foreman': drawSpeakerEntity(ctx, jfx, jfy, RADIUS, 0, 2 * Math.PI),
    };
    canvas.addEventListener('click', evt => {
      Object.entries(circlePaths).forEach(([key, circle]) => {
        if (ctx.isPointInPath(circle, evt.offsetX, evt.offsetY)) {
          if (currentCircle && currentCircle !== key) {
            ctx.fillStyle = 'white';
            ctx.fill(circlePaths[currentCircle]);
          }
          ctx.fillStyle = 'red';
          ctx.strokeStyle = 'red';
          ctx.fill(circle);
          ctx.fillStyle = 'white';
          currentCircle = key;
          setSpeakerAsActive(key);
        }
      });
    });
  },
  tick: function () {
    // const globalJurorAPosition = getChildEntityPosition('juror-a');
    // const globalJurorBPosition = getChildEntityPosition('juror-b');
    // const globalJurorCPosition = getChildEntityPosition('juror-c');
    // const globalJuryForemanPosition = getChildEntityPosition('jury-foreman');
  },
});
