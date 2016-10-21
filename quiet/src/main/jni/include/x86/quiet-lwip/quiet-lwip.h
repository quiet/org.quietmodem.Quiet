#include <quiet.h>

typedef struct {
    const quiet_encoder_options *encoder_opt;
    const quiet_decoder_options *decoder_opt;
    const char *hostname;
    unsigned int encoder_rate;
    unsigned int decoder_rate;
    uint8_t hardware_addr[6];
} quiet_lwip_driver_config;

typedef uint32_t quiet_lwip_ipv4_addr;

struct netif;
typedef struct netif quiet_lwip_interface;

ssize_t quiet_lwip_get_next_audio_packet(quiet_lwip_interface *interface, quiet_sample_t *buf, size_t samplebuf_len);

void quiet_lwip_recv_audio_packet(quiet_lwip_interface *interface, quiet_sample_t *buf, size_t samplebuf_len);

void quiet_lwip_init();

quiet_lwip_interface *quiet_lwip_create(quiet_lwip_driver_config *conf,
                                        quiet_lwip_ipv4_addr local_address,
                                        quiet_lwip_ipv4_addr netmask,
                                        quiet_lwip_ipv4_addr gateway);

quiet_lwip_interface *quiet_lwip_autoip(quiet_lwip_interface *interface);

void quiet_lwip_close(quiet_lwip_interface *interface);

void quiet_lwip_destroy(quiet_lwip_interface *interface);

void quiet_lwip_start_threads(quiet_lwip_interface *interface);
