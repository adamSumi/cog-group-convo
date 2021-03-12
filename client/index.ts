import {videoPlaybackSystem} from './systems/videoPlayback';
import {videoPlaybackComponent} from './components/videoPlayback';
import {captionComponent} from './components/caption';
import {indicatorComponent} from './components/indicator';
import {stayBelowComponent} from './components/stay-below';
import {rotateQEComponent} from './components/rotate-qe';

// Necessary so webpack won't mark these as dead code. Probably a better way to do this but ¯\_(ツ)_/¯
captionComponent;
indicatorComponent;
videoPlaybackSystem;
videoPlaybackComponent;
stayBelowComponent;
rotateQEComponent;
