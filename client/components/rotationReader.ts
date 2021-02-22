import {Entity, registerComponent} from 'aframe';
import { Object3D, Vector3 } from 'three';

const getChildEntityPosition = (childId: string): Vector3 => {
  const childEntity = document.querySelector(`#${childId}`);
  const childEntityPosition = childEntity?.object3D.position;
  const parentEntityPosition = (childEntity?.parentElement as Entity).object3D.position;
  return new Vector3().copy(childEntityPosition).add(parentEntityPosition);
}

export const rotationReaderComponent = registerComponent('rotation-reader', {
  tick: function () {
    const canvas: HTMLCanvasElement = document.getElementById('global-setup') as HTMLCanvasElement;
    const ctx = canvas.getContext('2d');
    ctx?.beginPath();
    ctx?.arc(10, 10, 10, 0, 2 * Math.PI);
    ctx?.stroke();

    const globalJurorAPosition = getChildEntityPosition('juror-a');
    const globalJurorBPosition = getChildEntityPosition('juror-b');
    const globalJurorCPosition = getChildEntityPosition('juror-c');
    const globalJuryForemanPosition = getChildEntityPosition('jury-foreman');
    // console.log({globalJurorAPosition, globalJurorBPosition, globalJurorCPosition, globalJuryForemanPosition})
  },
});
