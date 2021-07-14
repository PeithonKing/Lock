import numpy as np
import cv2

cap = cv2.ImageCapture(0)
zer = np.zeros(cap.shape, cap.dtype)


while True:
	ret, frame = cap.read()
	gray = cv2.addWeighted(frame, 2.5, zer, 0, 0)
	# cv2.imshow('frame', frame)
	cv2.imshow('gray', gray)
	if cv2.waitKey(1) == ord('q'):
		break

cap.release()
cv2.destroyAllWindows()
