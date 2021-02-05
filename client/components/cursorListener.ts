import {registerComponent} from 'aframe';

export const cursorListenerComponent = registerComponent('cursor-listener', {
  schema: {
    speakerId: {type: 'string'},
  },
  init: function () {
    const captionEl = document.querySelector('a-text#caption');
    const ambientCaptionEl = document.querySelector('a-text#ambientCaption');
    this.el.addEventListener('click', () => {
      // update caption's cursor target
      captionEl.setAttribute(
        'caption',
        `speaker: ${captionEl.getAttribute('caption').speaker}; cursorTarget: ${
          this.data.speakerId
        }; ambientCaption: ${captionEl.getAttribute('caption').ambientCaption};`
      );
      ambientCaptionEl.setAttribute(
        'caption',
        `speaker: ${
          ambientCaptionEl.getAttribute('caption').speaker
        }; cursorTarget: ${this.data.speakerId}; ambientCaption: ${
          ambientCaptionEl.getAttribute('caption').ambientCaption
        };`
      );
    });
    this.el.addEventListener('mouseleave', () => {
      // remove caption's cursor target
      captionEl.setAttribute(
        'caption',
        `speaker: ${
          captionEl.getAttribute('caption').speaker
        }; cursorTarget: ''; ambientCaption: ${
          captionEl.getAttribute('caption').ambientCaption
        };`
      );
      ambientCaptionEl.setAttribute(
        'caption',
        `speaker: ${
          ambientCaptionEl.getAttribute('caption').speaker
        }; cursorTarget: ''; ambientCaption: ${
          ambientCaptionEl.getAttribute('caption').ambientCaption
        };`
      );
    });
  },
});
