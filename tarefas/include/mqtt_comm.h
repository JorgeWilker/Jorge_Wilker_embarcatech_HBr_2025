/**
 * @file mqtt_comm.h
 * @brief Funções para comunicação MQTT básica
 */

#ifndef MQTT_COMM_H
#define MQTT_COMM_H

#include "lwip/apps/mqtt.h"
#include <stddef.h> // Para usar size_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configura e inicia a conexão MQTT com um broker.
 * 
 * @param client_id Identificador único para este cliente.
 * @param broker_ip Endereço IP do broker como string (ex: "192.168.1.1").
 * @param user Nome de usuário para autenticação (pode ser NULL).
 * @param pass Senha para autenticação (pode ser NULL).
 */
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass);

/**
 * @brief Publica dados em um tópico MQTT.
 * 
 * @param topic Nome do tópico (ex: "sensor/temperatura").
 * @param data Payload da mensagem (bytes).
 * @param len Tamanho do payload.
 */
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // MQTT_COMM_H 