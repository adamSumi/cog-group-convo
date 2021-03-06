import {Entity, registerComponent} from 'aframe';
import {Vector3} from 'three';

const INACTIVE_INDICATOR_COLOR = '#FFD700';
const ACTIVE_INDICATOR_COLOR = '#FF0000';

export const indicatorComponent = registerComponent('indicator', {
  schema: {speakerId: {type: 'string'}},

  init: function () {
    // @ts-ignore
    this.speakerWorldPosition = document
      .querySelector(`#${this.data.speakerId}`)
      .object3D.getWorldPosition(new Vector3());
  },
  rotateIndicator: function (indicator: Entity, position: Vector3) {
    indicator.object3D.lookAt(position);
    indicator.object3D.rotateX(90);
  },

  tick: function () {
    // @ts-ignore
    this.rotateIndicator(this.el, this.speakerWorldPosition);
  },
});
