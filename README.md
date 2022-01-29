# webServer
## 运行项目
```shell
./build.sh
nohup ./main &
```  

## update 2022.1.29
- [x] 使用shared_ptr管理多个类(WebServer HttpConnection timerManage)共享的Utils类  
- [x] 优化定时删除定时器的算法，降低时间复杂度 <img src="https://latex.codecogs.com/svg.image?O(n^2)&space;->&space;O(nlogn)" title="O(n^2) -> O(nlogn)" />
