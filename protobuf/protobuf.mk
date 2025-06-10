messages.pb.cc messages.pb.h: ../protobuf/messages.proto
	protoc -I=../protobuf --cpp_out=. ../protobuf/messages.proto

protobuf-clean:
	rm -f messages.pb.cc messages.pb.h
