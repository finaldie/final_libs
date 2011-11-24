//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tu_inc.h"
#include "../3rd/mongo-c-driver/src/mongo.h"

#define ASSERT assert

//TODO...

void	test_auth(){
	mongo_connection conn[1];
	
    if (mongo_connect( conn , "127.0.0.1", 27017 )){
        printf("failed to connect\n");
        exit(1);
    }
}

void	test_insert(){
	mongo_connection conn[1];
	
    if (mongo_connect( conn , "113.47.120.41", 27017 )){
        printf("failed to connect\n");
        exit(1);
    }

	bson b[1];
	bson_buffer buf[1];

	bson_buffer_init( buf );
	bson_append_string( buf, "pwd", "123" );
	bson_append_int( buf, "user_id", 100 );
	bson_from_buffer( b, buf );
	mongo_insert( conn, "test.final", b );

	bson_destroy( b );

	bson cond[1], op[1]; 
	bson_buffer cond_buf[1], op_buf[1];

	bson_buffer_init( cond_buf ); 
	bson_append_string( cond_buf, "name", "Joe"); 
	bson_append_int( cond_buf, "age", 33);
	bson_from_buffer( cond, cond_buf );
					
	bson_buffer_init( op_buf );
	bson_append_start_object( op_buf, "$inc");
	bson_append_int( op_buf, "visits", 1 );
	bson_append_finish_object( op_buf );
	bson_append_start_object( op_buf, "$set");
	bson_append_int( op_buf, "age", 34 );
	bson_append_finish_object( op_buf );
	bson_from_buffer( op, op_buf );
										
	mongo_update(conn, "test.final", cond, op, 0);
	bson_destroy( cond );
	bson_destroy( op );

    bson query[1];
	bson_buffer query_buf[1];
	mongo_cursor *cursor;

	bson_buffer_init( query_buf );
	bson_append_int( query_buf, "age", 34 );
	bson_from_buffer( query, query_buf );
				    
	// the mongo_find use the create_message interface , the head of msg has id field, fill that and will get it from responseTo field when read the response msg :)
	cursor = mongo_find( conn, "test.final", query, NULL, 0, 0, 0 );
	while( mongo_cursor_next( cursor ) ) {
	    bson_iterator it[1];
        if ( bson_find( it, &cursor->current, "name" )) {
	        printf( "name: %s\n", bson_iterator_string( it ) );
	    }
	}
	bson_destroy( query );

	mongo_destroy( conn );
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
