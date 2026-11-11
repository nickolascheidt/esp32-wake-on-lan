// Tampa tipo "capa" para a case amarela do ESP32 DevKit 30 pinos (WROOM-32, micro-USB).
// A saia veste a case por fora; um degrau interno apoia na borda da case e define a altura do teto.
// Modelado na posição de uso (saia para baixo) e girado 180° no final para imprimir com o teto na mesa.
// Medidas da case tiradas do 3MF original (ESP32-WROOM-32 Box, PLAUK).

/* [Peça a exportar] */
parte = "tampa";          // "tampa" ou "teste" (anel de 3 mm para validar a folga)

/* [Case existente] */
case_comp     = 56.8;     // comprimento externo (Y)
case_larg     = 33.1;     // largura externa (X)
case_alt      = 13.65;    // altura externa total, da mesa até a borda
case_raio     = 3.0;      // raio dos cantos externos
case_parede   = 2.0;      // espessura da parede da case
usb_larg      = 9.4;      // largura do recorte do micro-USB na case
usb_prof      = 3.45;     // profundidade do recorte a partir da borda
usb_lado      = -1;       // parede curta do USB: -1 = Y negativo, 1 = Y positivo (centralizado em X)

/* [Placa] */
pinos_acima_borda = 3.0;  // MEDIR: quanto a ponta dos pinos (ou o componente mais alto) passa da borda

/* [Tampa] */
folga         = 0.2;      // folga da saia por lado (atrito)
parede        = 2.0;      // espessura das paredes da tampa
teto          = 2.0;      // espessura do teto
folga_teto    = 2.0;      // folga entre o componente mais alto e o teto
altura_saia   = 7.0;      // quanto a saia desce por fora da case
chanfro       = 0.5;      // chanfro nas arestas externas
usb_extra_larg  = 2.0;    // recorte da saia mais largo que o da case
usb_extra_fundo = 1.0;    // e mais fundo

/* [Furos dos botões EN e BOOT] */
botao_d   = 3.0;
botao_x   = 7.5;          // distância do centro em X (um furo de cada lado do USB)
botao_y   = 6.0;          // distância da parede interna do lado do USB até o centro do furo

/* [Ventilação] */
vent_qtd     = 4;         // 0 desliga
vent_larg    = 1.5;
vent_comp    = 16;
vent_passo   = 4;
vent_y       = 8;         // centro do grupo de fendas em Y (lado oposto ao USB é positivo)

$fn = 64;

// ---------- derivadas ----------
saia      = min(altura_saia, case_alt - 1);       // requisito: nunca mais que a case - 1 mm
alt_int   = pinos_acima_borda + folga_teto;       // do topo da borda até o teto
H         = saia + alt_int + teto;
ext_comp  = case_comp + 2 * (folga + parede);
ext_larg  = case_larg + 2 * (folga + parede);
ext_raio  = case_raio + folga + parede;
int_comp  = case_comp - 2 * case_parede;
int_larg  = case_larg - 2 * case_parede;
int_raio  = max(case_raio - case_parede, 0.5);

assert(alt_int > 0, "pinos_acima_borda + folga_teto precisa ser > 0");
assert(vent_qtd == 0 || (vent_qtd - 1) * vent_passo / 2 + vent_y + vent_larg < int_comp / 2 - 1,
       "fendas de ventilação saem do teto");

module ret(c, l, r) { offset(r) square([l - 2 * r, c - 2 * r], center = true); }   // X = largura, Y = comprimento
module placa(c, l, r, z0, h) { translate([0, 0, z0]) linear_extrude(h) ret(c, l, r); }

module casca() {
    hull() {
        placa(ext_comp - 2 * chanfro, ext_larg - 2 * chanfro, ext_raio - chanfro, 0, 0.01);
        placa(ext_comp, ext_larg, ext_raio, chanfro, H - 2 * chanfro);
        placa(ext_comp - 2 * chanfro, ext_larg - 2 * chanfro, ext_raio - chanfro, H - 0.01, 0.01);
    }
}

module tampa() {
    difference() {
        casca();
        // saia: abraça a parede externa da case
        placa(case_comp + 2 * folga, case_larg + 2 * folga, case_raio + folga, -1, saia + 1);
        // espaço acima da borda: do tamanho do vão interno da case, então o degrau apoia na borda
        placa(int_comp, int_larg, int_raio, saia - 0.01, alt_int + 0.01);
        // janela do micro-USB, do teto até abaixo do recorte da case
        z_usb = saia - usb_prof - usb_extra_fundo;
        translate([-(usb_larg + usb_extra_larg) / 2, usb_lado > 0 ? int_comp / 2 - 1 : -ext_comp / 2 - 1, z_usb])
            cube([usb_larg + usb_extra_larg, (ext_comp - int_comp) / 2 + 2, saia + alt_int - z_usb]);
        // furos EN e BOOT
        for (sx = [-1, 1])
            translate([sx * botao_x, usb_lado * (int_comp / 2 - botao_y), H - teto - 1])
                cylinder(d = botao_d, h = teto + 2);
        // fendas de ventilação (atravessam X, empilhadas em Y)
        for (i = [0 : vent_qtd - 1]) if (vent_qtd > 0)
            translate([0, -usb_lado * vent_y + (i - (vent_qtd - 1) / 2) * vent_passo, H - teto / 2])
                hull() for (sx = [-1, 1])
                    translate([sx * (vent_comp - vent_larg) / 2, 0, 0]) cylinder(d = vent_larg, h = teto + 2, center = true);
    }
}

// anel de teste: só o perfil da saia, 3 mm
module teste() {
    difference() {
        placa(ext_comp, ext_larg, ext_raio, 0, 3);
        placa(case_comp + 2 * folga, case_larg + 2 * folga, case_raio + folga, -1, 5);
    }
}

// impressão: teto na mesa, saia para cima
if (parte == "teste") teste();
else translate([0, 0, H]) rotate([180, 0, 0]) tampa();
