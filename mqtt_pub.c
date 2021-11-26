#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mosquitto.h>

int count = 0;
time_t start;

#define QOS 2
#define MAX 200000
#define INFLIGHT 1000
#define SIZE 1024
#define DEBUG false;


void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	time_t elapsed = time(NULL) - start;
	count++;
	if (!(count % 100)) {
		printf("Published %d %ld m/s \n", count, count/elapsed);
	}
}


int main(){
	int rc;
	struct mosquitto * mosq;
	char payload[SIZE];

	/* fill the sample message */
	memset(payload, '-', SIZE);
	payload[SIZE] = 0;

	mosquitto_lib_init();

	mosq = mosquitto_new("publisher-test", true, NULL);

	rc = mosquitto_int_option(mosq, MQTT_PROTOCOL_V311, 0);
/*	rc = mosquitto_int_option(mosq, MOSQ_OPT_TCP_NODELAY, 1); */
	rc = mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
	if(rc != 0){
		printf("Client could not set protocol version Error Code: %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}

	rc = mosquitto_int_option(mosq, MOSQ_OPT_SEND_MAXIMUM, INFLIGHT);
	if(rc != 0){
		printf("Client could not set messages max Error Code: %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}

/*
	rc = mosquitto_threaded_set(mosq, 1);
	if (rc != MOSQ_ERR_SUCCESS) {
		printf("Failed to tune the thread model for libmosquitto (%s)", mosquitto_strerror(rc));
	}
*/
	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	if(rc != 0){
		printf("Client could not connect to broker! Error Code: %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}
	printf("We are now connected to the broker!\n");

	start = time(NULL);

	for (int i=0; i<MAX; i++) {
		if (DEBUG) printf("pub %d len %ld\n", i, strlen(payload));
		mosquitto_publish(mosq, NULL, "test/t1", strlen(payload), payload, QOS, false);
	}

	mosquitto_publish_callback_set(mosq, my_publish_callback);

/*
	mosquitto_loop_start(mosq);
	printf("Press Enter to quit...\n");
	getchar();
	mosquitto_loop_stop(mosq, true);
*/
	while (count < MAX) {
		if (DEBUG) printf(".");
		mosquitto_loop(mosq, 1, INFLIGHT*10);
	}
	printf("Elapsed: %ld sec \n", time(NULL) - start);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();
	return 0;
}
