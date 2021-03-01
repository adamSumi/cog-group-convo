import {videoPlaybackSystem} from './systems/videoPlayback';
import {videoPlaybackComponent} from './components/videoPlayback';
import {captionComponent} from './components/caption';

// Necessary so webpack won't mark these as dead code. Probably a better way to do this but ¯\_(ツ)_/¯
captionComponent;
videoPlaybackSystem;
videoPlaybackComponent;
