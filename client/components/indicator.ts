import {Component, Entity, registerComponent} from 'aframe';
import {Vector3} from 'three';

export const INACTIVE_INDICATOR_COLOR = '#FFD700';
const ACTIVE_INDICATOR_COLOR = '#FF0000';

const toRadians = (degrees: number): number => degrees * (Math.PI / 180);

export const indicatorComponent = registerComponent('indicator', {
  schema: {
    jurorId: {type: 'string'},
    speakerVec: {type: 'vec3', default: new Vector3()},
    captionEl: {type: 'selector', default: '#caption'},
    anchorVec: {type: 'vec3', default: new Vector3()},
  },

  tick: function () {
    const captionEl = this.data.captionEl;
    if (captionEl.components.caption.getSpeaker() === '') {
      return;
    }
    if (captionEl.components.caption.getSpeakerIsActive()) {
      this.el.setAttribute('color', ACTIVE_INDICATOR_COLOR);
    } else {
      this.el.setAttribute('color', INACTIVE_INDICATOR_COLOR);
    }
    document
      .querySelector(`#${this.data.jurorId}`)
      .object3D.getWorldPosition(this.data.speakerVec);

    // Rotate indicator
    const anchorEl = document.querySelector(`#anchor`); // Anchor element, positioned in front of chest
    // Step 1: Make anchor look at target
    anchorEl.object3D.lookAt(this.data.speakerVec);
    // Step 2: Copy anchor's rotation to this element's rotation
    this.el.object3D.rotation.copy(anchorEl.object3D.rotation);
    // Step 3: Rotate cone slightly.
    this.el.object3D.rotateX(toRadians(-45));
  },
});
