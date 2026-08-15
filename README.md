# gRPC Demo

C++23 / gRPC / Bazel 기반 학습 프로젝트.  
Unary RPC와 Server Streaming RPC 패턴을 구현하고, 테스트와 coverage까지 갖춘 예제.

## 환경

| 도구 | 버전 |
|------|------|
| Bazel | 8.4.2 |
| GCC | 14.2.0 |
| Clang (tidy) | 20.1.1 |
| gRPC | 1.76.0 |
| Protobuf | 29.3 |
| GoogleTest | 1.15.2 |

모든 `make` 명령은 내부적으로 `module add`를 수행하므로 별도 환경 설정 불필요.

## 프로젝트 구조

```
grpc/
├── proto/                      ← gRPC 서비스 정의 (protobuf)
│   └── pingpong.proto
├── server/
│   ├── pingpong_service.h/cc   ← 서비스 로직 (cc_library)
│   ├── server.cc               ← 서버 실행 바이너리 (cc_binary)
│   └── test/
│       └── pingpong_test.cc    ← 서버 로직 테스트
├── client/
│   ├── pingpong_client.h/cc    ← 클라이언트 로직 (cc_library)
│   ├── client.cc              ← 클라이언트 실행 바이너리 (cc_binary)
│   └── test/
│       └── pingpong_client_test.cc  ← 클라이언트 로직 테스트
├── util/
│   ├── sum/                    ← 유틸리티 예제 (합계)
│   │   └── test/
│   └── factorial/              ← 유틸리티 예제 (팩토리얼)
│       └── test/
├── main/                       ← 기본 app 바이너리
├── tools/                      ← clang-tidy 설정
├── Makefile                    ← 빌드/테스트/coverage 진입점
├── MODULE.bazel                ← Bazel 의존성 정의
└── .bazelrc                    ← 컴파일러, 캐시, output 설정
```

### 설계 원칙

- 프로덕션 로직은 `cc_library`로 분리하고, 바이너리(`cc_binary`)와 테스트(`cc_test`) 모두 같은 library를 의존
- 테스트 파일은 각 모듈의 `test/` 하위 디렉토리에 배치
- coverage는 실제 프로덕션 코드만 측정 (테스트 코드 제외)

## 구현된 예제

### 1. Ping/Pong (Unary RPC)

클라이언트가 메시지를 보내면 서버가 "pong: {메시지}"로 응답하는 단순 요청-응답 패턴.

```protobuf
rpc Ping (PingRequest) returns (PongResponse) {}
```

- 동기 호출: `PingPongClient::Ping()`
- 비동기 호출: `PingPongClient::AsyncPing()` (콜백 기반)

### 2. Subscribe (Server Streaming RPC)

클라이언트가 구독하면 서버가 상태 알림(Indication)을 실시간으로 push하는 패턴.

```protobuf
rpc Subscribe (SubscribeRequest) returns (stream StatusIndication) {}
```

- 서버: `PingPongServiceImpl::Publish()`로 모든 구독자에게 동시 전달
- 클라이언트: stream에서 blocking read로 수신
- 여러 클라이언트가 동시 구독 가능

## 빌드

```bash
# 전체 빌드
make build

# compile_commands.json 생성 (VSCode clangd용)
make compdb
```

## 실행

```bash
# 터미널 1: 서버 실행
make server

# 터미널 2: 클라이언트 실행
make client
```

## 테스트

```bash
# 전체 테스트
make test

# 특정 타겟만
make test TARGET=//server/test:pingpong_test

# 특정 테스트 케이스만
make test TARGET="//server/test:pingpong_test --test_arg=--gtest_filter=PingPongTest.MultipleSubscribers*"
```

### 테스트 목록

| 타겟 | 내용 |
|------|------|
| `//server/test:pingpong_test` | Ping 동작, Subscribe/Indication 수신, 다중 구독자 |
| `//client/test:pingpong_client_test` | 클라이언트 동기/비동기 호출 검증 |
| `//util/sum/test:sum_test` | 합계 함수 |
| `//util/factorial/test:factorial_test` | 팩토리얼 함수 |

## Coverage

```bash
make coverage
# 결과: coverage/index.html 에서 확인
```

프로젝트 소스 코드(`//util`, `//server`, `//client`, `//main`)만 측정하며, 외부 의존성(gRPC, protobuf 등)은 제외.

## 정적 분석

```bash
make tidy
```

clang-tidy를 이용한 코드 품질 검사.

## 캐시

- **Disk cache**: `/local/edooyou/bazel-cache` — 빌드 결과 로컬 캐싱 (gRPC/protobuf 재컴파일 방지)
- **Output base**: `/local/edooyou/bazel-output-base` — NFS 성능 문제 회피

## 정리

```bash
make clean
```
