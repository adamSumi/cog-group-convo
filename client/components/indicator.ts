import {Component, Entity, registerComponent} from 'aframe';
import {Vector3} from 'three';

const INACTIVE_INDICATOR_COLOR = '#FFD700';
const ACTIVE_INDICATOR_COLOR = '#FF0000';

export const indicatorComponent = registerComponent('indicator', {
  schema: {
    speakerVec: {type: 'vec3', default: new Vector3()},
    captionEl: {type: 'selector', default: '#caption'},
  },

  rotateIndicator: function (indicator: Entity, position: Vector3) {
    indicator.object3D.lookAt(position);
    indicator.object3D.rotateX(90);
  },

  // The event emission that causes the indicator to shrink over time is fired in videoPlayback.ts:L95
  tick: function () {
    const captionEl = this.data.captionEl;
    if (captionEl.components.caption.getSpeakerIsActive()) {
      this.el.setAttribute('color', ACTIVE_INDICATOR_COLOR);
    } else {
      this.el.setAttribute('color', INACTIVE_INDICATOR_COLOR);
    }
    document
      .querySelector(`#${captionEl.components.caption.getSpeaker()}`)
      .object3D.getWorldPosition(this.data.speakerVec);
    this.rotateIndicator(this.el, this.data.speakerVec);
  },
});
