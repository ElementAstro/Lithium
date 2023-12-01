import { useEffect } from 'react';

function LoadingAnimation() {
  useEffect(() => {
    function fadeOutLoadingAnimation() {
      setTimeout(() => {
        document.getElementById('loading-animation').style.display = 'none';
      }, 500);
    }

    if (window.jQuery) {
      $(document).ready(() => {
        fadeOutLoadingAnimation();
      });
    } else {
      document.onreadystatechange = () => {
        if (document.readyState === 'interactive') {
          fadeOutLoadingAnimation();
        }
      };
    }
  }, []);

  return (
    <div id="loading-animation">
      <div id="loading-animation-center">
        <div id="loading-animation-center-absolute">
          <div className="loading_object" id="loading_four"></div>
          <div className="loading_object" id="loading_three"></div>
          <div className="loading_object" id="loading_two"></div>
          <div className="loading_object" id="loading_one"></div>
        </div>
      </div>
    </div>
  );
}

export default LoadingAnimation;
