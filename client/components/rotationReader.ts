import { Entity, registerComponent } from 'aframe';
import { Vector3 } from 'three';

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
    `speaker: ${captionEl.getAttribute('caption').speaker
    }; activeTarget: ${speaker}; ambientCaption: ${captionEl.getAttribute('caption').ambientCaption
    }`
  );
  ambientCaptionEl.setAttribute(
    'caption',
    `speaker: ${ambientCaptionEl.getAttribute('caption').speaker
    }; activeTarget: ${speaker}; ambientCaption: ${ambientCaptionEl.getAttribute('caption').ambientCaption
    };`
  );
};

export const rotationReaderComponent = registerComponent('rotation-reader', {
  init: function () {
    const keyboardMap: Map<string, string> = new Map([['1', 'juror-a'], ['2', 'juror-b'], ['3', 'juror-c'], ['4', 'jury-foreman']]);

    document.addEventListener('keydown', (event) => {
      if (keyboardMap.has(event.key)) {
        const jurorCode = keyboardMap.get(event.key)!;
        setSpeakerAsActive(jurorCode);
      }
    })
  },
  tick: function () {
    // const globalJurorAPosition = getChildEntityPosition('juror-a');
    // const globalJurorBPosition = getChildEntityPosition('juror-b');
    // const globalJurorCPosition = getChildEntityPosition('juror-c');
    // const globalJuryForemanPosition = getChildEntityPosition('jury-foreman');
  },
});
