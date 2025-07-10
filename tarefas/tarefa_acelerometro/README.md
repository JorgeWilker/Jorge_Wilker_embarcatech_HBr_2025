# 📱 Tarefa: Acelerômetro MPU-6050 com Display OLED

👤 **Autor**: Jorge Wilker Mamede de Andrade  
📍 **Data**: 30 de Junho de 2025  
🎓 **Curso**: Residência Tecnológica em Sistemas Embarcados - EmbarcaTech  
🏢 **Instituição**: EmbarcaTech - HBr  

## 📝 Descrição da Tarefa
Esta tarefa implementa a leitura de dados do sensor MPU-6050 (acelerômetro e giroscópio) usando Raspberry Pi Pico com exibição simultânea no terminal serial e display OLED SSD1306 via comunicação I2C.

## 🚀 Como Usar
1. **Conexões físicas:**
   - MPU-6050: SDA→GPIO0, SCL→GPIO1, VCC→3V3, GND→GND
   - OLED SSD1306: SDA→GPIO14, SCL→GPIO15, VCC→3V3, GND→GND

2. **Compilar e carregar:**
   ```bash
   mkdir "C:\tarefa_acelerometro_bitdoglab\build"
   cd "C:\tarefa_acelerometro_bitdoglab\build"
   cmake ..
   cmake --build .
   ```

3. **Executar:**
   - Conecte o Pico em modo BOOTSEL
   - Copie o arquivo `.uf2` gerado para o dispositivo
   - Abra terminal serial em 115200 baud para visualizar dados

## 📋 Requisitos
- Raspberry Pi Pico SDK configurado
- CMake (versão 3.13+)
- GCC ARM Embedded Toolchain
- MPU-6050 e OLED SSD1306
- Terminal serial para visualização de dados

## 📂 Arquivos da Tarefa
```
tarefa_acelerometro_bitdoglab/
├── src/main.c                   # Código principal da tarefa
├── include/                     # Bibliotecas OLED SSD1306
├── CMakeLists.txt              # Configuração de build
├── README.md                   # Este arquivo
└── LICENSE                     # Licença GPL-3.0
```

## 🎯 Resultado Esperado
- Dados do acelerômetro e giroscópio (X, Y, Z) exibidos no terminal a cada 1 segundo
- Visualização simultânea dos mesmos dados no display OLED 128x64
- Valores próximos a ±16384 para 1g no acelerômetro
- Valores próximos a zero no giroscópio quando em repouso

## 📜 Licença
GPL-3.0 - Consulte o arquivo [LICENSE](LICENSE) para detalhes.

---
*Tarefa desenvolvida para o curso EmbarcaTech 2025* 