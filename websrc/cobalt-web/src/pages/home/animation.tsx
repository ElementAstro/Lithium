import React, { useEffect, useState } from "react";
import { GlobalStore } from "../../store/globalStore";
import "./style.less";
import { useNavigate } from "react-router-dom";
import { RingLoader } from "react-spinners";
import styled from "styled-components";

function Welcome() {
  const navigate = useNavigate();

  const [animationDone, setAnimationDone] = useState(false);
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    async function fetchData() {
      try {
        await GlobalStore.actions.connect.getProfileList();
        await GlobalStore.actions.GlobalParameterStore.get_all_paramters();
        await GlobalStore.actions.connect.getProfileDevice();
        setIsLoading(false);
      } catch (error) {
        console.error(error);
        navigate("/error?error=500&message='Server not found'");
      }
    }
    fetchData();
  }, []);

  useEffect(() => {
    setTimeout(() => {
      setAnimationDone(true);
    }, 1000);
  }, []);

  useEffect(() => {
    if (animationDone && !isLoading) {
      setTimeout(() => {
        navigate("/home");
      }, 2000);
    }
  }, [animationDone, isLoading]);

  const SpinnerContainer = styled.div`
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
  `;

  const Spinner = styled(RingLoader)`
    display: block;
    margin: 0 auto;
    border-color: red;
  `;

  return (
    <div>
      {isLoading && (
        <SpinnerContainer>
          <Spinner size={150} color="#3f51b5" loading={isLoading} />
        </SpinnerContainer>
      )}
      <div
        style={{
          backgroundColor: "#FFFFFF",
          position: "absolute",
          width: "100%",
          height: "100%",
          top: 0,
          left: 0,
        }}
      ></div>
      <img
        className={`opening-animation ${animationDone ? "fade-out" : ""}`}
        src="/background.png"
        alt="Welcome"
        style={{
          position: "absolute",
          width: "100%",
          height: "100%",
          top: 0,
          left: 0,
        }}
      />
    </div>
  );
}

export default Welcome;
