import {registerComponent} from 'aframe';

export const captionComponent = registerComponent('caption', {
  schema: {
    speaker: {type: 'string'},
    activeTarget: {type: 'string'},
    ambientCaption: {type: 'boolean'},
  },
  init: function () {
    // should be empty
  },
  update: function () {
    if (!this.data.speaker || !this.data.activeTarget) {
      return;
    }
    let opacity: number;
    const cursorIsOnSpeaker = this.data.speaker === this.data.activeTarget;
    if (this.data.ambientCaption) {
      // If this is an ambient captioning element, we want it only to appear when not on the correct speaker.
      opacity = !cursorIsOnSpeaker ? 1 : 0;
    } else {
      opacity = cursorIsOnSpeaker ? 1 : 0;
    }
    this.el.setAttribute('opacity', opacity);
  },
});
