import {registerComponent} from 'aframe';

const INACTIVE_INDICATOR_COLOR = '#FFD700';
const ACTIVE_INDICATOR_COLOR = '#FF0000';

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

const displayTriangleOnPhone = (speakerId: string, isActive: boolean) => {
  const speakerTriangle = document.querySelector(
    `a-triangle[speaker-id=${speakerId}]`
  );
  if (isActive) {
    speakerTriangle.setAttribute('color', ACTIVE_INDICATOR_COLOR);
  }
  speakerTriangle.emit('fade');
};

export const captionComponent = registerComponent('caption', {
  schema: {
    speaker: {type: 'string'},
    activeTarget: {type: 'string'},
    ambientCaption: {type: 'boolean'},
  },
  init: function () {
    const keyboardMap: Map<string, string> = new Map([
      ['1', 'juror-a'],
      ['2', 'juror-b'],
      ['3', 'juror-c'],
      ['4', 'jury-foreman'],
    ]);

    document.addEventListener('keydown', event => {
      if (keyboardMap.has(event.key)) {
        const jurorCode = keyboardMap.get(event.key)!;
        setSpeakerAsActive(jurorCode);
      }
    });
  },
  update: function () {
    if (!this.data.speaker || !this.data.activeTarget) {
      return;
    }
    const speakerIsActive = this.data.speaker === this.data.activeTarget;
    if (this.data.speaker !== '') {
      console.log('speaker is', this.data.speaker);
      document.querySelectorAll('a-triangle').forEach(elem => {
        elem.setAttribute('color', INACTIVE_INDICATOR_COLOR);
        elem.setAttribute('material', 'opacity', 0);
      });
      displayTriangleOnPhone(
        this.data.speaker,
        speakerIsActive
      );
    }
    let opacity: number;
    if (this.data.ambientCaption) {
      // If this is an ambient captioning element, we want it only to appear when not on the correct speaker.
      opacity = !speakerIsActive ? 1 : 0;
    } else {
      opacity = speakerIsActive ? 1 : 0;
    }
    this.el.setAttribute('opacity', opacity);
  },
});
