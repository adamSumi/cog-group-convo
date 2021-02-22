import {Entity, registerComponent} from 'aframe';
import {Object3D, Vector3} from 'three';

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
  startAngle: number = 0,
  endAngle: number = 2 * Math.PI
) => {
  ctx.beginPath();
  ctx.fillStyle = '#FFFFFF';
  ctx.strokeStyle = '#FFFFFF';
  ctx.beginPath();
  ctx.arc(x, y, radius, startAngle, endAngle);
  ctx.closePath();
  ctx.fill();
};

export const rotationReaderComponent = registerComponent('rotation-reader', {
  tick: function () {
    const canvas: HTMLCanvasElement = document.getElementById(
      'global-setup'
    ) as HTMLCanvasElement;
    const ctx = canvas.getContext('2d')!;
    drawSpeakerEntity(ctx, 30, 120, 30, 0, 2 * Math.PI);
    drawSpeakerEntity(ctx, 120, 30, 30, 0, 2 * Math.PI);
    drawSpeakerEntity(ctx, 210, 30, 30, 0, 2 * Math.PI);
    drawSpeakerEntity(ctx, 300, 120, 30, 0, 2 * Math.PI);

    const globalJurorAPosition = getChildEntityPosition('juror-a');
    const globalJurorBPosition = getChildEntityPosition('juror-b');
    const globalJurorCPosition = getChildEntityPosition('juror-c');
    const globalJuryForemanPosition = getChildEntityPosition('jury-foreman');
    // console.log({globalJurorAPosition, globalJurorBPosition, globalJurorCPosition, globalJuryForemanPosition})
  },
});
