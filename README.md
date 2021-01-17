# OpenGL 볼륨 렌더링 프로젝트

<br>

> [Visual Studio에서 OpenGL 설치하기](https://blog.amaorche.com/20)  <br>
> 3차원 좌표와 벡터를 저장, 연산자 오버로딩을 구현한 gmath.h, gmath.cpp는 직접 작성한 코드가 아닙니다. <br>


<br>
<br>

 ## Volume Rendering (볼륨 렌더링)
 
 <br>

 <img src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcS3_M-zudjENf9CbeugyMWSwZSreiGN4Hh8Vg&usqp=CAU" width="500"/>
 
 <br>
 
[사진 출처](https://arxiv.org/pdf/1812.04233.pdf)

<br>
<br>

> 3차원 스칼라 장 형태의 이산 샘플링 데이터를 2차원 투시로 보여주는 기술 <br>


<br>
<br>

 ## 입력
 
 <br>

> `data\bighead.den` 의 볼륨 데이터에서 생성한 단면 <br>
> `SliceRender` 프로젝트를 실행하여 확인 <br>

<br>
 
 <img src="https://j.gifs.com/lxZW37.gif"/>
 
<br>
<br>

 ## 결과
 
 <br>
 
 > `VolRender` 프로젝트를 실행하여 확인 <br>
 
 <br>
 
 <img width="500" alt="정면" src="https://user-images.githubusercontent.com/31186176/104835206-62e89a00-58e8-11eb-888c-d5f8251f5772.PNG">
<img width="500" alt="옆면" src="https://user-images.githubusercontent.com/31186176/104835200-5cf2b900-58e8-11eb-820b-71d167bd3def.PNG">
<img width="500" alt="윗면" src="https://user-images.githubusercontent.com/31186176/104835203-5feda980-58e8-11eb-883b-60e1d3dede0e.PNG">

<br>
<br>
 
 ## 과정
 
 <br>
 
 - **그레디언트**
 
 <br>
 
 <img src="https://user-images.githubusercontent.com/31186176/104835620-341ff300-58eb-11eb-87b2-c58630007763.png" width="500"/>
 
 <br>
 
 > 법선 벡터(normal) <br>
 
 <br>
 <br>
 
 - **분할**
 
 <br>
 
 <img src="https://user-images.githubusercontent.com/31186176/104835667-79dcbb80-58eb-11eb-86f0-74ac4f772e52.png" width="500"/>
 
 <br>
 
  > 불투명도(opacity) <br>
  > Transfer 함수 사용 <br>
 
 <br>
 <br>
 
 - **쉐이딩**
 
 <br>
 
 <img src="https://user-images.githubusercontent.com/31186176/104835684-8eb94f00-58eb-11eb-929f-fc1f06f64f9b.png" width="500"/>
 
 <br>
 
 > 간략화한 phong 쉐이딩 사용 <br>
 
 <br>
 <br>
 
 - **합성**
 
 <br>
 
 <img src="https://user-images.githubusercontent.com/31186176/104835821-8c0b2980-58ec-11eb-99fc-b06856ead388.png" width="500"/>
 
 <br>
 
 > 샘플링을 위한 [삼선형 보간법](https://darkpgmr.tistory.com/117) 사용 <br>
 
 <br>
 <br>
