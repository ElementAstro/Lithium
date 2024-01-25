import styled from "styled-components";

export const Body = styled.body`
  font-family: Segoe UI, sans-serif;
  background: #3973aa;
  color: #fefeff;
  height: 100vh;
  margin: 0;
`;

export const Page = styled.div`
  display: table;
  height: 100%;
  margin: 0 auto;
  margin-top: -10px;
  width: 70%;
  font-size: 1.9vw;
`;

export const Container = styled.div`
  display: table-cell;
  vertical-align: middle;
`;

export const H1 = styled.h1`
  font-weight: normal;
  padding: 0;
  margin: 25px 0;
  margin-top: 0;
  font-size: 6rem;
  margin-bottom: 10px;
`;

export const H2 = styled.h2`
  font-weight: normal;
  padding: 0;
  margin: 25px 0;
  margin-top: 0;
  font-size: 21px;
`;

export const H4 = styled.h4`
  font-weight: normal;
  padding: 0;
  margin: 25px 0;
  margin-top: 0;
  font-size: 18px;
  line-height: 1.5em;
`;

export const H5 = styled.h5`
  line-height: 22px;
  font-size: 14px;
  font-weight: 400;
`;

export const Details = styled.div`
  display: flex;
  flex-flow: row;
  flex-wrap: nowrap;
  padding-top: 10px;
`;

export const QR = styled.div`
  flex: 0 1 auto;
`;

export const Image = styled.div`
  background: white;
  padding: 5px;
  line-height: 0;

  img {
    width: 9.8em;
    height: 9.8em;
  }
`;

export const StopCode = styled.div`
  padding-left: 10px;
  flex: 1 1 auto;
`;

export const Link = styled.a`
  color: #e1f0ff;
`;