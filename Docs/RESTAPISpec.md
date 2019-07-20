# REST API v0.0

## Game Properties

| Description | HTTP Verb | Path |
| :---------- | :-------- | :--- |
| List all available component types | GET | /Components |
| List all available system types | GET | /Systems |
| List all loaded worlds | GET | /Worlds |
| Create a world | POST | /Worlds/\<**World**> |
| Delete a world | DELETE | /Worlds/\<**World**> |

## Entity Properties

| Description | HTTP Verb | Path |
| :---------- | :-------- | :--- |
| List all components on an entity | GET | /Worlds/\<**World**>/Entities/\<**ID**>/Components |
| Read component by type | GET | /Worlds/\<**World**>/Entities/\<**ID**>/Components/\<**Type**> |
| ID of the entity | GET | /Worlds/\<**World**>/Entities?Name=\<**Name**>|
| Modify a component | PUT | /Worlds/\<**World**>/Entities/\<**ID**>/Components/\<**Component**> |
| Modify a single value of a component | PATCH | /Worlds/\<**World**>/Entities/\<**ID**>/Components/\<**Component**>?Field=<**Field**> |
| Change the name of an entity | PATCH | /Worlds/\<**World**>/Entities/\<**ID**> |
| Add a component | PUT | /Worlds/\<**World**>/Entities/\<**ID**>/Components/<**Component**> |
| Remove a component | DELETE | /Worlds/\<**World**>/Entities/\<**ID**>/Components/<**Component**> |
| List systems in a world | GET | /Worlds/\<**World**>/Systems |
| List systems applying logic to an Entity | GET | /Worlds/\<**World**>/Systems?EntityID=\<**ID**> |

## World Properties

| Description | HTTP Verb | Path |
| :---------- | :-------- | :--- |
| List all entities in a world | GET | /Worlds/\<**World**>/Entities |
| List all systems in a world | GET | /Worlds/\<**World**>/Systems |
| Create an Entity | POST | /Worlds/\<**World**>/Entities |
| Delete an Entity | DELETE | /Worlds/\<**World**>/Entities |
| Add a system | PUT | /Worlds/\<**World**>/Systems/\<**System**> |
| Remove a System | DELETE | /Worlds/\<**World**>/Systems/\<**System**> |

