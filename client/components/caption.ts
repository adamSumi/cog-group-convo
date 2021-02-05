import {registerComponent} from 'aframe';

export const captionComponent = registerComponent('caption', {
  schema: {
    speaker: {type: 'string'},
    cursorTarget: {type: 'string'},
    ambientCaption: {type: 'boolean'},
  },
  init: function () {
    // should be empty
  },
  update: function () {
    if (!this.data.speaker || !this.data.cursorTarget) {
      return;
    }
    console.log(`Ambient caption is: ${this.data.ambientCaption}`)
    let opacity: number;
    const cursorIsOnSpeaker = this.data.speaker === this.data.cursorTarget;
    if (this.data.ambientCaption) {
      // If this is an ambient captioning element, we want it only to appear when not on the correct speaker.
      opacity = !cursorIsOnSpeaker ? 1 : 0;
    } else {
      opacity = cursorIsOnSpeaker ? 1 : 0;
    }
    this.el.setAttribute('opacity', opacity);
  },
});
