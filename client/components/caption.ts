import {Entity, registerComponent} from 'aframe';
import {Vector3} from 'three';

const INACTIVE_INDICATOR_COLOR = '#FFD700';
const ACTIVE_INDICATOR_COLOR = '#FF0000';

const getChildEntityPosition = (childId: string): Vector3 => {
  const childEntity = document.querySelector(`#${childId}`);
  const childEntityPosition = childEntity?.object3D.position;
  const parentEntityPosition = (childEntity?.parentElement as Entity).object3D
    .position;
  return new Vector3().copy(childEntityPosition).add(parentEntityPosition);
};

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

const rotateIndicator = (
  indicator: Entity,
  camera: Entity,
  target: Vector3
) => {
  indicator.object3D.lookAt(target);
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
  tick: function () {
    const camera = document.querySelector('#camera');
    document.querySelectorAll('a-cone').forEach(cone => {
      const speakerId = cone.getAttribute('speaker-id');
      const correspondingSpeakerVideoPosition = getChildEntityPosition(
        speakerId
      );
      rotateIndicator(
        cone as Entity,
        camera,
        correspondingSpeakerVideoPosition
      );
    });
  },
  update: function () {
    if (!this.data.speaker || !this.data.activeTarget) {
      return;
    }
    const speakerIsActive = this.data.speaker === this.data.activeTarget;
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
