import {Entity, registerComponent} from 'aframe';

const ROTATION_ANGLE = 5 * (Math.PI / 180);
export const rotateQEComponent = registerComponent('rotate-qe', {
  init: function () {
    document.addEventListener('keydown', event => {
      switch (event.key) {
        case 'q':
          this.el.object3D.rotateY(ROTATION_ANGLE);
          break;
        case 'e':
          this.el.object3D.rotateY(-ROTATION_ANGLE);
          break;
      }
    });
  },
});
