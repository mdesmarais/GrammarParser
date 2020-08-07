
#define ASSERT_EQUALS(msg, x, y, format) do { \
    if ((x) != (y)) {                     \
        log_error((format), (msg), (x), (y)); \
        abort(); \
    } \
} while(0)

#define ASSERT_CHAR_EQUALS(msg, x, y) ASSERT_EQUALS(msg, x, y, "%s -> expected=%c got=%c")
#define ASSERT_INT_EQUALS(msg, x, y) ASSERT_EQUALS(msg, x, y, "%s -> expected=%d got=%d")

#define ASSERT_FALSE(msg, cond) ASSERT_INT_EQUALS(msg, false, cond)
#define ASSERT_TRUE(msg, cond) ASSERT_INT_EQUALS(msg, true, cond)

#define ASSERT_STR_EQUALS(msg, str1, str2) do { \
    size_t _l1 = strlen((str1));                  \
    size_t _l2 = strlen((str2));                  \
                                                \
    if (_l1 != _l2 || strcmp((str1), (str2)) != 0) {         \
        log_error("%s -> expected=%s got=%s", (msg), (str1), (str2)); \
        abort(); \
    }                                           \
} while(0);

#define ASSERT_PTR_EQUALS(msg, x, y) ASSERT_EQUALS(msg, x, y, "%s -> expected=%p got=%p")

#define ASSERT_LIST_EQUALS(msg, l1, l2, comparator) do { \
    if ((l1).size != (l2).size) {          \
        log_error("%s -> different list size expected=%ld got=%ld", (msg), (l1).size, (l2).size); \
        abort(); \
    }                                        \
                                             \
    ll_LinkedListItem *_item1 = (l1).front; \
    ll_LinkedListItem *_item2 = (l2).front; \
                                             \
    while (_item1 && _item2) {               \
        if ((comparator)(_item1->data, _item2->data) != 0) {  \
            log_error("%s -> expected=%p got=%p", (msg), _item1->data, _item2->data);               \
            abort(); \
        }                                    \
        _item1 = _item1->next;               \
        _item2 = _item2->next;               \
    }                                        \
} while(0);
