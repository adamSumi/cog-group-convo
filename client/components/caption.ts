import {Entity, registerComponent} from 'aframe';
import {Vector3} from 'three';

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

const displayArrowOnPhone = (
  speakerPosition: Vector3,
  phonePosition: Vector3
) => {
  const phoneArrow = new Vector3();
  phoneArrow.setX(speakerPosition.x - phonePosition.x);
  phoneArrow.setY(0);
  phoneArrow.setZ(speakerPosition.z - phonePosition.z);
  console.log('Before normalization', phoneArrow);
  const normalizedPhoneArrow = phoneArrow.normalize();
  const scaledPhoneArrow = normalizedPhoneArrow.multiplyScalar(0.3);
  console.log('After normalization and scaling', scaledPhoneArrow);
  const startPoint = new Vector3().copy(phonePosition);
  const endPoint = new Vector3().copy(phonePosition).add(scaledPhoneArrow);
  const thickLine = document.createElement('a-entity');
  thickLine.setAttribute(
    'meshline',
    `lineWidth: 20; path:${startPoint.x} ${startPoint.y} ${startPoint.z},${endPoint.x} ${endPoint.y} ${endPoint.z}; color: #FFD700`
  );
  thickLine.setAttribute('class', 'thickline');
  document.querySelector('a-scene').appendChild(thickLine);
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
    if (this.data.speaker !== '') {
      console.log('speaker is', this.data.speaker);
      const phonePosition = document.querySelector('#smartphone').object3D
        .position;
      const speakerPosition = getChildEntityPosition(this.data.speaker);
      document
        .querySelectorAll('.thickline')
        .forEach(elem => elem.parentNode?.removeChild(elem));
      displayArrowOnPhone(speakerPosition, phonePosition);
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
