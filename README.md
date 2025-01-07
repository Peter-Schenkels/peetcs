# peetcs  (WIP)

A simple, flexible and performant ECS.  

---

## Features  

- **Archetype-Based:** Cache efficient grouping of entities for fast access.  
- **Flexibility:** Creates archetype containers of any set of data types/components at runtime.  

---

## Quick Start  

### 1. Include Headers  

```cpp
#include "peetcs.hpp"
```

### 2. Define Components  

```cpp
struct position { float x, y, z; };
struct health { int points; };
struct cool_data { float test; };
```

### 3. Manage Components  

#### Add Components:  
```cpp
peetcs::archetype_pool pool;
entity_id entity = 0;

position& pos = pool.add<position>(entity);  
pos.x = 1; pos.y = 2; pos.z = 3;

health& hp = pool.add<health>(entity);  
hp.points = 100;

cool_data& cd = pool.add<cool_data>(entity);  
cd.test = 43;

pool.emplace_commands();  // Apply changes (invalidates previous references)
```

#### Remove Components:  
```cpp
pool.remove<health>(entity);  
pool.emplace_commands();
```

#### Query Components:  
```cpp
auto query = pool.query<position, health>();

for (auto q : query) {
    position& pos = q.get<position>();
    health& hp = q.get<health>();
    std::cout << pos.x << " " << pos.y << " " << pos.z << "\n";
}
```

---

## Notes  

- **`emplace_commands()` invalidates references.** Always re-query after calling this function.  
- Queries are efficient once all changes are applied.
- Might contain bugs still very WIP
---
