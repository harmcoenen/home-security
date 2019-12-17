#include "emailMessage.h"

// constructor
emailMessage::emailMessage(const int numDetections,detectNet::Detection* detections) {
    printf("\nEMAIL: %i objects detected", numDetections);
    printf("\nEMAIL: detected obj class #%u, confidence=%f", detections[0].ClassID, detections[0].Confidence);
}

// destructor
emailMessage::~emailMessage() {
}
