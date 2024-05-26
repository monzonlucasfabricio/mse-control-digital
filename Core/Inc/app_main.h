/*
 * app.h
 *
 *  Created on: May 19, 2024
 *      Author: Lucas
 */

#ifndef INC_APP_MAIN_H_
#define INC_APP_MAIN_H_

#include "main.h"
#include "task.h"

typedef enum{
	API_OK,
	API_ERROR,
	API_INVALID
}retType;

retType APP_TasksCreate(void);


#endif /* INC_APP_MAIN_H_ */
