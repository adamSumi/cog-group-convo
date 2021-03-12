import {Entity, registerComponent} from 'aframe';

export const stayBelowComponent = registerComponent('stay-below', {
  tick: function () {
    const cameraEntity: Entity = document.querySelector('#camera');
    this.el.object3D.position.copy(cameraEntity.object3D.position).setY(0.7);
  },
});
