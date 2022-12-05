#ifdef NDEBUG
;
#else

char const * list_fail_get_data_from_sentinel	= "%s: Tried to get data from sentinel\n";
char const * list_fail_add_entry_after     		= "%s: Tried to add after a nonexistent List entry\n";
char const * list_fail_add_entry_before    		= "%s: Tried to add before a nonexistent List entry\n";
char const * list_fail_delete_entry        		= "%s: Tried to delete a nonexistent List entry\n";
char const * list_fail_delete_entry_by_pointer	= "%s: Tried to delete-by-pointer the sentinel of a list.\n";
char const * list_fail_alter_entry         		= "%s: Tried to alter a nonexistent List entry\n";
char const * list_fail_next_entry_nonexist 		= "%s: Tried to get entry after nonexistent entry.";
char const * list_fail_next_entry_sentinel 		= "%s: Tried to get next entry, which is sentinel.";
char const * list_fail_prev_entry_nonexist 		= "%s: Tried to get entry before nonexistent entry";
char const * list_fail_prev_entry_sentinel 		= "%s: Tried to get previous entry, which is sentinel.";
char const * list_fail_last_entry		 		= "%s: Tried to get last entry from a 0-length list.\n.";
char const * list_fail_first_entry		 		= "%s: Tried to get first entry from a 0-length list.\n.";
char const * list_fail_similar_entry       		= "%s: Tried to get entry similar to nonexistent entry";
char const * list_fail_delete_last_entry   		= "%s: Tried to delete last element from an empty List.\n";
char const * list_fail_delete_first_entry  		= "%s: Tried to delete first element from an empty List.\n";
char const * list_fail_operator            		= "%s: Tried to get entry %d from %d-entry list.\n";
char const * lit_fail_next                 		= "%s: Tried to take an Iterator past the sentinel of a List.\n";
char const * lit_fail_operator             		= "%s: Tried to take contents of sentinel of List\n";
char const * lit_fail_delete_current       		= "%s: Tried to delete sentinel of List\n";
char const * lit_fail_change_current       		= "%s: Tried to change contents of sentinel of List\n";

#endif


